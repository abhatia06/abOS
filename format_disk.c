#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include "kernel/stdint.h"
#include "kernel/fs/fs.h"

// Here's the basic plan:
// Somehow I need to either grab/generate an img file that is 1.44MB large, just like our actual disk image
// Initialize/build superblock
// Initialize/build bootblock
// Initialize/build bitmaps (inode & data)
// Initialize/build inode block
// Initialize/build data blocks

FILE* disk_ptr = 0;

superblock_t superblock = {0};
uint32_t disk_size = 512*2880;  // keeping it to be 1.44MB for right now
uint32_t num_files = 0;
uint32_t file_blocks = 0;
uint32_t first_block = 0;
uint32_t next_inode_id = 4;     // 0 for invalid, 1 for root dir, 2 for prekernel, 3 for kernel

uint32_t bytes_to_blocks(uint32_t bytes) {
        uint32_t file_size_bytes = bytes;
        uint32_t file_size_sectors = file_size_bytes/FS_SECTOR;
        if(file_size_bytes%FS_SECTOR > 0) {
                file_size_sectors++;
        }

        uint32_t file_size_blocks = file_size_sectors/SECTORS_PER_BLOCK;
        if(file_size_sectors%SECTORS_PER_BLOCK > 0) {
                file_size_blocks++;
        }

        return file_size_blocks;
}

bool write_boot_block() {
        boot_record_t boot_block = {0};         // check fs.h, boot_record just allocates 4096 bytes (1 block)

        FILE* file_ptr = fopen("build/boot.bin", "rb");
        if(!file_ptr) {
                fprintf(stderr, "something went wrong with BOOTLOADER");
                return false;
        }

        int check = fread(boot_block.sector[0], FS_SECTOR, 1, file_ptr);
        if(check != 1) {
                fprintf(stderr, "something went wrong with READING bootloader");
                return false;
        }

        // 2nd stage

        file_ptr = fopen("build/bootstage2.bin", "rb");
        if(!file_ptr) {
                fprintf(stderr, "something went wrong with 2ND STAGE BOOTLOADER");
                return false;
        }

        for(int i = 1; i < 7; i++) {
                check = fread(boot_block.sector[i], FS_SECTOR, 1, file_ptr);
                if(check == 0) {

                        // 0 out remaining memory of the 4KB block (unnecessary b/c boot_block = {0}, but whatever)
                        memset(boot_block.sector[i], 0, FS_SECTOR);
                        for(int j = i + 1; j < 7; j++) {
                                memset(boot_block.sector[j], 0, FS_SECTOR);
                        }
                        break;
                }
        }

        fclose(file_ptr);
        check = fwrite(&boot_block, FS_BLOCK, 1, disk_ptr);
        if(check == 1) {
                return true;
        }
        return false;
}

bool get_file_info(char* dir_path) {
        DIR* dir_ptr = opendir(dir_path);
        if(!dir_ptr) {
                fprintf(stderr, "couldn't open directory %s\n", dir_path);
                return EXIT_FAILURE;
        }

        uint32_t dir_entries = 2;

        struct dirent* dir_ent = readdir(dir_ptr);
        while(dir_ent) {
                if(strncmp(dir_ent->d_name, ".", 2) == 0 || strncmp(dir_ent->d_name, "..", 3) == 0) {
                        dir_ent = readdir(dir_ptr);
                        continue;
                }

                char* buffer = malloc(strlen(dir_path) + strlen(dir_ent->d_name) + 2);
                sprintf(buffer, "%s/%s", dir_path, dir_ent->d_name);

                if(dir_ent->d_type == DT_DIR) {
                        printf("directory %s\n", buffer);
                }
                else {
                        printf("file %s\n", buffer);
                }

                num_files++;
                dir_entries++;

                struct stat file_stat;
                if(stat(buffer, &file_stat) < 0) {
                        printf("couldn't get stat for %s\n", buffer);
                        return false;
                }

                if(dir_ent->d_type == DT_REG) {
                        file_blocks += bytes_to_blocks(file_stat.st_size);
                }
                else {
                        if(!get_file_info(buffer)) {
                                return false;
                        }
                }

                free(buffer);
                dir_ent = readdir(dir_ptr);
        }

        closedir(dir_ptr);
        file_blocks += bytes_to_blocks(dir_entries * sizeof(dir_entry_t));
        return true;
}

bool write_superblock() {
        superblock.num_inodes = num_files + 2;  // exclude inode 0 (invalid) and inode 1 (root dir)
        superblock.num_inodes_bitmap = superblock.num_inodes / (FS_BLOCK * 8); // 1 byte = 8 bits
        if(superblock.num_inodes % (FS_BLOCK * 8)) {
                superblock.num_inodes_bitmap++;         // partial block takes up entire block
        }
        superblock.num_inodes_per_block = FS_BLOCK/sizeof(inode_t);
        superblock.num_inodes_per_sector = FS_SECTOR/sizeof(inode_t);
        superblock.first_inode_bitmap_block = 2;
        superblock.first_data_bitmap_block = superblock.first_inode_bitmap_block + superblock.num_inodes_bitmap;
        superblock.inode_size = sizeof(inode_t);                // 64 bytes, (hopefully)
        superblock.num_inode_blocks = superblock.num_inodes/superblock.num_inodes_per_block;

        uint32_t disk_blocks = bytes_to_blocks(disk_size);

        // stole this from queso fuego, don't fully understand why it works, as it doesn't seem to actually take into
        // account the number of blocks the data bitmap takes up, but whatever
        uint32_t data_blocks = (disk_blocks - superblock.first_data_bitmap_block - superblock.num_inode_blocks - 1);
        superblock.num_data_bitmap = data_blocks/(FS_BLOCK * 8);
        if(data_blocks % (FS_BLOCK * 8)) {
                superblock.num_data_bitmap++;
        }
        superblock.first_inode_block = superblock.first_data_bitmap_block + superblock.num_data_bitmap;
        superblock.first_data_block = superblock.first_inode_block +
                (superblock.num_inodes/superblock.num_inodes_per_block);
        superblock.num_datablocks = superblock.first_data_block + file_blocks;
        superblock.max_file_size = 0xFFFFFFFF;          // 32 bit max size
        superblock.root_i_number = 0;                   //pre-kernel or kernel will fill this out
        superblock.first_free_inode_bit = superblock.num_inodes;
        superblock.first_free_data_bit = superblock.first_data_block + file_blocks;
        superblock.device_number = 0x1;

        int count = fwrite(&superblock, sizeof(superblock_t), 1, disk_ptr);
        if(count != 1) {
                return false;
        }
        if(sizeof(superblock_t) < FS_BLOCK) {   // which it should be, superblock_t is 128 bytes
                uint8_t temp_buffer[FS_BLOCK] = {0};
                count = fwrite(&temp_buffer, FS_BLOCK - sizeof(superblock_t), 1, disk_ptr);
                if(count == 1) {
                        return true;
                }
                return false;
        }
        return true;
}

// 1 = free inode bit, 0 = not free inode bit, so for initialization we set everything to 1. Easiest way to do that is
// by setting 32-bit chunks of memory as 0xFFFFFFFF
bool write_inode_bitmap() {
        // arguably not the best, as I set the entirety of the inode bitmap to be 1, when I shouldn't really be doing that, but it's whatever (hopefully)
        uint32_t chunk;
        uint32_t count;
        uint32_t size = superblock.num_inode_bitmap * FS_BLOCK;
        size = size/4;          // right now it's in bytes, we divide by 4 to make it in 32-bit chunks

        chunk = 0xFFFFFFF0;     // reserve inode 0 for invalid inode, inode 1 for root dir, inode 2 for prekernel, 3 for kernel
        count = fwrite(&chunk, 4, 1, disk_ptr);
        if(count != 1) {
                printf("error setting first chunk to be 1\n");
                return false;
        }

        for(int i = size + 1; i > 0; i--) {
                chunk = 0xFFFFFFFF;
                count = fwrite(&chunk, 4, 1, disk_ptr);         // write 4 bytes at a time
                if(count != 1) {
                        printf("error setting chunk to 1 at chunk %d\n", i);
                        return false;
                }
        }

        return true;
}

bool write_data_bitmap() {
        uint32_t chunk;
        uint32_t count;
        uint32_t size = superblock.num_data_bitmap * FS_BLOCK;
        size = size/4;

        chunk = 0xFFFFFFFE;     // this time just reserve first data chunk for prekernel
        count = fwrite(&chunk, 4, 1, disk_ptr);
        if(count != 1) {
                printf("error setting first chunk to 1 (DATA)\n");
                return false;
        }
        for(int i = size + 1; i > 0; i--) {
                chunk = 0xFFFFFFFF;
                count = fwrite(&chunk, 4, 1, disk_ptr);
                if(count != 1) {
                        printf("error setting chunks to 1 at chunk %d (DATA)\n", i);
                        return false;
                }
        }
        return true;

}

bool write_file_data(char* dir_path, uint32_t curr_inode, uint32_t parent_inode) {
        uint32_t count;
        DIR* dir_ptr = opendir(dir_path);
        if(!dir_ptr) {
                fprintf(stderr, "couldn't open directory %s\n", dir_path);
                return EXIT_FAILURE;
        }

        // remember, we haven't set up anything yet for the root inode, so we can't assume "." and ".." exist
        uint32_t dir_entries = 0;
        struct dirent* dir_ent = readdir(dir_ptr);
        while(dir_ent) {
                dir_entries++;
                dir_ent = readdir(dir_ptr);
        }

        // rewind the pointer for later use, (to read through each file and add it to the dir)
        //rewinddir(dir_ptr);

        uint32_t dir_size = dir_entries * sizeof(dir_entry_t);

        inode_t dir_inode = (inode_t){0};
        dir_inode.i_number = curr_inode;
        dir_inode.file_type = FILE_TYPE_DIR;
        dir_inode.size = dir_size;
        dir_inode.time_created = (fs_time_t) {
                .second = 0,
                .minute = 40,
                .hour = 15,
                .day = 28,
                .month = 7,
                .year = 2025,
        };

        for(int i = 0; i < bytes_to_blocks(dir_size); i++) {
                dir_inode.direct_pointers[i] = first_block;
                first_block++;
        }

        // go to the specific part of the disk img that corresponds to the i_number
        // go to the block (byte address) that houses the inode we want to edit
        fseek(disk_ptr, (superblock.first_inode_block + (dir_inode.i_number / INODES_PER_BLOCK)) * FS_BLOCK, SEEK_SET);
        // go to the actual address (use modulo as basically indexing into the block and seek_cur to add onto the current pointer)
        fseek(disk_ptr, (dir_inode.i_number%INODES_PER_BLOCK) * sizeof(inode_t), SEEK_CUR);

        // write our new inode into that specific part of the disk
        count = fwrite(&dir_inode, sizeof(dir_inode), 1, disk_ptr);
        if(count != 1) {
                return false;
        }

        // for "." and ".." dir entries (current and parent)
        fseek(disk_ptr, dir_inode.direct_pointers[0] * FS_BLOCK, SEEK_SET);

        dir_entry_t dir_entry = (dir_entry_t){0};
        dir_entry.i_number = dir_inode.i_number;
        strcpy(dir_entry.name, ".");
        count = fwrite(&dir_entry, sizeof(dir_entry), 1, disk_ptr);
        if(count != 1) {
                return false;
        }

        dir_entry.i_number = parent_inode;
        strcpy(dir_entry.name, "..");
        count = fwrite(&dir_entry, sizeof(dir_entry), 1, disk_ptr);
        if(count != 1) {
                return false;
        }

        // now we need to add the file data for all the files
        // some of this next code is gonna be just copy and pasted from get_file_info
        rewinddir(dir_ptr);
        dir_ent = readdir(dir_ptr);
        while(dir_ent) {
                // skip past "." and "..". Also, remember there is an invisible "\0" char at the end of each string
                if(strncmp(dir_ent->d_name, ".", 2) == 0 || strncmp(dir_ent->d_name, "..", 3) == 0) {
                        dir_ent = readdir(dir_ptr);
                        continue;
                }

                char* buffer = malloc(strlen(dir_path) + strlen(dir_ent->d_name) + 2);
                sprintf(buffer, "%s/%s", dir_path, dir_ent->d_name);

                if(strncmp(dir_ent->d_name, "prekernel.bin", 13) == 0) {
                        dir_entry.i_number = 2; // look at write_inode_bitmap, we reserve inode 2 for prekernel
                } else if(strncmp(dir_ent->d_name, "kernel.bin", 11) == 0) {
                        dir_entry.i_number = 3;
                }
                else {
                        dir_entry.i_number = next_inode_id;
                        next_inode_id++;
                }

                strcpy(dir_entry.name, dir_ent->d_name);

                // add in the directory entries
                // we dont have to do seek for the correct area this time, because we already did fseek beforehand for
                // making "." and "..", and so the files should come right after "." and ".."
                count = fwrite(&dir_entry, sizeof(dir_entry), 1, disk_ptr);
                if(count != 1) {
                        return false;
                }

                // Now that we have the directory entries in the directory, all we have to do is add in the files
                // themselves. Because right now, if we have, for example, inode 20 inside this dir, that inode 20
                // may be claimed in the bitmap, but on the inode block, it doesn't have anything to point to. We need
                // to do that now.

                // save current position in dir for the next dir_entry in the path
                uint32_t cur_dir_pos = ftell(disk_ptr);

                // now switch gears into reading files/directories
                struct stat file_stat;
                if(stat(buffer, &file_stat) < 0) {
                        fprintf(stderr, "couldn't get stat for %s\n", buffer);
                        return false;
                }

                // https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
                if(dir_ent->d_type == DT_REG) {

                        // this code is basically copy and pasted from what we did earlier in the function
                        inode_t new_file = (inode_t){0};
                        new_file.i_number = dir_entry.i_number;
                        new_file.file_type = FILE_TYPE_FILE;
                        new_file.size = file_stat.st_size;
                        new_file.time_created = (fs_time_t) {
                                .second = 6,
                                .minute = 13,
                                .hour = 18,
                                .day = 28,
                                .month = 7,
                                .year = 2025,
                        };

                        for(int i = 0; i < bytes_to_blocks(file_stat.st_size); i++) {
                                new_file.direct_pointers[i] = first_block;
                                first_block++;
                        }

                        fseek(disk_ptr, (superblock.first_inode_block + (new_file.i_number / INODES_PER_BLOCK)) * FS_BLOCK, SEEK_SET);
                        fseek(disk_ptr, (new_file.i_number%INODES_PER_BLOCK) * sizeof(inode_t), SEEK_CUR);
                        count = fwrite(&new_file, sizeof(new_file), 1, disk_ptr);
                        if(count != 1) {
                                return false;
                        }

                        // this is the part where this code differs. We were already editing the data blocks before,
                        // but now we need to go and add in the file data to the data blocks.

                        // go to the first data block
                        fseek(disk_ptr, new_file.direct_pointers[0] * FS_BLOCK, SEEK_SET);

                        FILE* file_ptr = fopen(buffer, "rb");
                        if(!file_ptr) {
                                fprintf(stderr, "couldn't open %s\n", buffer);
                                return false;
                        }

                        uint32_t size_sectors = new_file.size/FS_SECTOR;
                        if(new_file.size%FS_SECTOR > 0) {
                                size_sectors++;
                        }

                        uint8_t sector_buffer[FS_SECTOR] = {0};
                        uint32_t total_bytes = 0;
                        for(int i = 0; i < size_sectors; i++) {
                                uint32_t bytes = fread(sector_buffer, 1, FS_SECTOR, file_ptr);
                                count = fwrite(sector_buffer, 1, bytes, disk_ptr);
                                if(count != bytes) {
                                        printf("failed to read LINE 384 (OR AROUND THERE IDK)\n");
                                        return false;
                                }
                                total_bytes += bytes;
                        }
                        fclose(file_ptr);
                        uint8_t null_block[FS_BLOCK] = {0};
                        count = fwrite(null_block, 1, (FS_BLOCK - (total_bytes % FS_BLOCK)) % FS_BLOCK, disk_ptr);
                        if(count != (FS_BLOCK - (total_bytes % FS_BLOCK)) % FS_BLOCK) {
                                return false;
                        }

                        printf("Wrote file %s, %llu (%u blocks)\n", buffer, (unsigned long long)file_stat.st_size,
                                        bytes_to_blocks(file_stat.st_size));
                }
                else if(dir_ent->d_type == DT_DIR) {
                        if(!write_file_data(buffer, dir_entry.i_number, dir_inode.i_number)) {
                                return false;
                        }
                }
                else {
                        // idk lol
                }

                // restore current position so we can add next dir_entry
                fseek(disk_ptr, cur_dir_pos, SEEK_SET);
                dir_ent = readdir(dir_ptr);
        }

        uint8_t second_null_block[FS_BLOCK] = {0};
        closedir(dir_ptr);
        count = fwrite(second_null_block, 1, (FS_BLOCK - (dir_size % FS_BLOCK)) % FS_BLOCK, disk_ptr);
        if(count != (FS_BLOCK - (dir_size % FS_BLOCK)) % FS_BLOCK) {
                return false;
        }

        printf("Wrote directory %s, %u\n", dir_path, dir_size);
        return true;
}

// we need to set up the root inode and add all the files to exist under the root inode
bool init_inode_data_blocks() {
        uint32_t count = 0;
        first_block = superblock.first_data_block;

        if(!write_file_data("build/bin", 1, 1))  {
                return false;
        }

        uint32_t disk_blocks = bytes_to_blocks(disk_size);

        fseek(disk_ptr, 0, SEEK_END);
        uint32_t diff = disk_blocks = bytes_to_blocks(ftell(disk_ptr));

        uint8_t null_block[FS_BLOCK] = {0};
        for(int i = 0; i < diff; i++) {
                count = fwrite(null_block, FS_BLOCK, 1, disk_ptr);
                if(count != 1) {
                        return false;
                }
        }
        return true;
}

int main() {
        disk_ptr = fopen("build/os-imagetest.img", "wb");
        if(!disk_ptr) {
                fprintf(stderr, "Couldn't get the disk image %s\n", "build/os-imagetest.img");
                return EXIT_FAILURE;
        }
        printf("OPENED DISK IMAGE %s\n", "build/os-image.img");

        if(!get_file_info("build/bin")) {
                fprintf(stderr, "no files in build/bin");
                return EXIT_FAILURE;
        }
        printf("GOT FILE INFO FROM build/bin\n");

        if(!write_boot_block()) {
                fprintf(stderr, "boot block error");
                return EXIT_FAILURE;
        }
        printf("BOOT BLOCK LOADED\n");

        if(!write_superblock()) {
                fprintf(stderr, "superblock error");
                return EXIT_FAILURE;
        }
        printf("SUPERBLOCK LOADED\n");

        if(!write_inode_bitmap()) {
                fprintf(stderr, "inode bitmap error");
                return EXIT_FAILURE;
        }
        printf("INODE BITMAP LOADED\n");
        
        if(!write_data_bitmap()) {
                fprintf(stderr, "data bitmap error");
                return EXIT_FAILURE;
        }
        printf("DATA BITMAP LOADED\n");

        if(!init_inode_data_blocks()) {
                fprintf(stderr, "initialization inode/data blocks error");
                return EXIT_FAILURE;
        }
        printf("INODE BLOCKS & DATA BLOCKS INITIALIZED\n");

        fprintf("DISK FORMATTED (hopefully)\n");
        fclose(disk_ptr);
        
}
