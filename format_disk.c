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

superblock_t superblock = (superblock_t){0};
uint32_t disk_size = 512*2880;  // keeping it to be 1.44MB for right now
uint32_t num_files = 0;
uint32_t file_blocks = 0;

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

bool write_boot_block() {
        return false;
}

bool write_superblock() {
        return false;
}


int main() {
        disk_ptr = fopen("build/os-imagetest.img", "wb");
        if(!disk_ptr) {
                fprintf(stderr, "Couldn't get the disk image %s\n", "build/os-imagetest.img");
                return EXIT_FAILURE;
        }

        if(!get_file_info("build/bin")) {
                fprintf(stderr, "no files in build/bin");
                return EXIT_FAILURE;
        }

        if(!write_boot_block) {
                fprintf(stderr, "boot block error");
                return EXIT_FAILURE;
        }
}
