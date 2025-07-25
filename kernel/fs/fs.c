#include "fs.h"
#include "../stdint.h"
#include "../stdio.h"
#include "../x86.h"
#include "../util/string.h"

#define READ 0x20
#define WRITE 0x30

//  These will be initialized in the pre-kernel
inode_t current_dir_inode = {0};        // inode of the current directory we're in
superblock_t superblock = {0};          // the superblock (there's only 1)
int block_buffer[FS_BLOCK] = {0};       // a buffer to hold 1 block (4096)
int sector_buffer[FS_SECTOR] = {0};     // a buffer to hold 1 sector (512)
inode_t root_inode = {0};               // inode of the root directory

// uses ATA PIO LBA mode to read/write sectors to and from the disk
void rw_sectors(uint32_t sectors, uint32_t starting_sector, uint32_t address, int readwrite) {
        // In LBA mode, the starting sector (known as the LBA, or Logical Block Address) contains
        // the sector number, drive selector, and cylinder information, like so:
        //
        // SECTOR NUMBER        LBA[0:7]
        // CYLINDER LOW         LBA[15:8]
        // CYLINDER HIGH        LBA[23:16]
        // DRIVE/HEAD           LBA[27:24]
        outb(0x1F6, (0xE0 | ((starting_sector >> 24) & 0x0F)));
        outb(0x1F2, sectors);
        outb(0x1F3, ((starting_sector >> 8) & 0xFF));
        outb(0x1F4, ((starting_sector >> 16) & 0xFF));
        outb(0x1F7, readwrite);

        uint16_t* address_ptr = (uint16_t*)address;
        if(readwrite == READ) {
                for(uint32_t i = 0; i < sectors; i++) {
                        while(inb(0x1F7) & (1 << 7)) {
                                // poll
                        }

                        for(uint32_t j = 0; j < 512; j++) {
                                *address_ptr++ = inb(0x1F0);
                        }

                        // 400ns delay. (https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays)
                        for(int z = 0; z < 15; z++) {
                                inb(0x3F6);
                        }
                }
        }

        else if(readwrite == WRITE) {
                for(uint32_t i = 0; i < sectors; i++) {
                        while(inb(0x1F7) & (1 << 7)) {
                                // poll
                        }

                        for(uint32_t j = 0; j < 512; j++) {
                                outb(0x1F0, *address_ptr++);
                        }

                        // 400ns delay
                        for(int z = 0; z < 15; z++) {
                                inb(0x3F6);
                        }
                }

                outb(0x1F7, 0xE7);
                while(inb(0x1F7) & (1 << 7)) {
                        //poll
                }
        }
}


// assume that inode is initialized, and its direct block pointers do in fact point to existing data blocks
bool load_file(inode_t* inode, uint32_t address) {
        uint32_t file_size_bytes = inode->size;
        uint32_t file_size_sectors = file_size_bytes/FS_SECTOR;
        if(file_size_bytes%FS_SECTOR > 0) {
                file_size_sectors++;
        }

        uint32_t direct_blocks_to_read = file_size_sectors/8;   // 4096/512 = 8 sectors per block
        if(file_size_sectors%8 > 0) {
                direct_blocks_to_read++;
        }

        uint32_t offset;
        // we assume num of direct blocks to read is < 3, b/c I only have 3 direct blocks lol
        for(uint32_t i = 0; i < direct_blocks_to_read; i++) {
                rw_sectors(SECTORS_PER_BLOCK, inode->direct_pointers[i]*SECTORS_PER_BLOCK, address + offset, READ);
                offset+=4096;   // add 4KiB to offset, b/c each block is 4KiB
        }

        return true;
}

// again assume inode is initialized properly. This time we write to the direct blocks
bool save_file(inode_t* inode, uint32_t address) {
        uint32_t file_size_bytes = inode->size;
        uint32_t file_size_sectors = file_size_bytes/FS_SECTOR;
        if(file_size_bytes%FS_SECTOR > 0) {
                file_size_sectors++;
        }

        uint32_t direct_blocks_to_read = file_size_sectors/SECTORS_PER_BLOCK;   // 4096/512 = 8 sectors per block
        if(file_size_sectors%SECTORS_PER_BLOCK > 0) {
                direct_blocks_to_read++;
        }

        uint32_t offset;
        // we assume num of direct blocks to read is < 3, b/c I only have 3 direct blocks lol
        for(uint32_t i = 0; i < direct_blocks_to_read; i++) {
                rw_sectors(SECTORS_PER_BLOCK, inode->direct_pointers[i]*SECTORS_PER_BLOCK, address + offset, WRITE);
                offset+=4096;   // add 4KiB to offset, b/c each block is 4KiB
        }

        return true;
}


/*
 * Similar to linux, our files will all be directory entries into a directory that itself will have a root inode. Each directory will contain the following:
 * '.' - which will be the file name in a directory entry that will allow us to access the i_number for the current directory
 * '..' - which will be the file name in a directory entry that will allow us to access the i_number for the parent directory
 * Afterwards, everything else will be whatever the user puts into the directory, like for example:
 *  'foo.txt' - which will be the file name in a directory entry that exists inside the current directory (will also allow us to access the i_number for it)
 */
inode_t get_inode_in_dir(inode_t current_dir, char* file) {
        dir_entry_t* dir_entry = 0;

        //TODO: make converting from inode->size to sectors/blocks a function
        uint32_t file_size_bytes = current_dir.size;
        uint32_t file_size_sectors = file_size_bytes/FS_SECTOR;
        if(file_size_bytes%FS_SECTOR > 0) {
                file_size_sectors++;
        }

        uint32_t direct_blocks_to_read = file_size_sectors/SECTORS_PER_BLOCK;
        if(file_size_sectors%SECTORS_PER_BLOCK > 0) {
                direct_blocks_to_read++;
        }

        for(uint32_t i = 0; i < direct_blocks_to_read; i++) {
                rw_sectors(SECTORS_PER_BLOCK, i*SECTORS_PER_BLOCK, (uint32_t)block_buffer, READ);

                dir_entry = (dir_entry_t*)block_buffer;
                for(uint32_t j = 0; j < DIR_ENTRIES_PER_BLOCK; j++, dir_entry++) {
                        if(dir_entry->i_number == 0) {
                                continue;       // empty entry, skip. i_number = 0 reserved for empty i_node
                        }

                        if(strcmp(dir_entry->name, file) == 0) {
                                rw_sectors(1, inode_sector(dir_entry->i_number, superblock),
                                                (uint32_t)sector_buffer, READ);
                                inode_t* inode_array_in_sector = (inode_t*)sector_buffer;
                                return inode_array_in_sector[dir_entry->i_number % INODES_PER_SECTOR];
                        }
                }

        }
        return (inode_t){0};
}

// get the inode of a file given a path to the file
inode_t get_inode(char* path) {

        char* index = path;
        char str_buffer[60];
        inode_t current_inode;

        // in linux, the path root dir starts with /. In windows, it is C:\.
        if(*index == '/') {
                index++;
                current_inode = root_inode;
        }

        // if the path doesn't begin with '/', we assume we're in a directory
        else {
                current_inode = current_dir_inode;
        }

        int count_buffer = 0;
        int str_index = 0;
        while(*index != '\0') {
                if(*index == '/') {
                        str_buffer[str_index] = '\0';
                        current_inode = get_inode_in_dir(current_inode, str_buffer);
                        if(current_inode.i_number == 0) {
                                return (inode_t){0};
                        }
                        memset(str_buffer, 0, sizeof(str_buffer));
                        str_index = 0;
                }

                else {
                        if(str_index < sizeof(str_buffer) - 1) {
                                str_buffer[str_index] = *index;
                                str_index++;
                        }
                        else {
                                // too long (WILL NOT HAPPEN, since names of files are at most 60 characters, and
                                // our buffer is 60 characters)
                                return (inode_t){0};
                        }
                }
                index++;
        }

        // while loop doesn't process the last piece b/c it reaches null terminator (ex: /foo/bar/file.txt), so we
        // have to do it manually here
        str_buffer[str_index] = '\0';
        current_inode = get_inode_in_dir(current_inode, str_buffer);
        if(current_inode.i_number == 0) {
                return (inode_t){0};
        }

        return current_inode;
}

inode_t get_parent_inode(char* path) {

        char* temp = path;
        int index = strrchr(path, '/') - path;

        if(!strrchr(path, '/')) {
                return current_dir_inode;
        }

        temp[index] = '\0';

        if(strlen(temp) == 0) {
                return root_inode;
        }

        return get_inode(temp);
}

// for editing bitmap stuff
void set_inode_bitmap(uint32_t bit, bool set) {
        rw_sectors(SECTORS_PER_BLOCK, (superblock.first_inode_bitmap + (bit / 32768))*SECTORS_PER_BLOCK,
                        (uint32_t)block_buffer, READ);
        uint32_t index = bit % 32768;   // 32768 = 8 * 4096. 8 bits = 1 byte, a block is 4096 bytes, or 32768 bits

        uint64_t* chunk = (uint64_t*)block_buffer;      // take a big chunk and sift through it
        if(set) {
                chunk[index/64] |= (1 << (index % 64));
        }
        else {
                chunk[index/64] &= ~(1 << (index % 64));
        }

        // save back to disk
        rw_sectors(SECTORS_PER_BLOCK, (superblock.first_inode_bitmap + (bit/32768))*SECTORS_PER_BLOCK,
                        (uint32_t)block_buffer, WRITE);

}

void set_data_bitmap(uint32_t bit, bool set) {

        // probably would've been better to make this code apart of a helper function, but its whatever
        rw_sectors(SECTORS_PER_BLOCK, (superblock.first_data_bitmap + (bit / 32768))*SECTORS_PER_BLOCK,
                        (uint32_t)block_buffer, READ);
        uint32_t index = bit % 32768;

        uint64_t* chunk = (uint64_t*)block_buffer;
        if(set) {
                chunk[index/64] |= (1 << (index % 64));
        }
        else {
                chunk[index/64] &= ~(1 << (index % 64));
        }

        rw_sectors(SECTORS_PER_BLOCK, (superblock.first_data_bitmap + (bit/32768))*SECTORS_PER_BLOCK,
                        (uint32_t)block_buffer, WRITE);

}

char* get_name_path(char* path) {
        char* temp = path;
        char* temp2 = path;
        while(*temp != '\0') {
                if(*temp == '/') {
                        temp2 = temp + 1;
                }
                temp++;
        }
        return temp2;
}

void update_superblock() {
        *(superblock_t*)SUPERBLOCK_ADDRESS = superblock;

        rw_sectors(1, SUPERBLOCK_DISK_SECTOR, SUPERBLOCK_ADDRESS, WRITE);
}

void update_inode(inode_t inode) {
        rw_sectors(1, (superblock.first_inode_block * SECTORS_PER_BLOCK) +
                        (inode.i_number / INODES_PER_SECTOR), (uint32_t)sector_buffer, READ);

        inode_t* temp = (inode_t*)sector_buffer + (inode.i_number % INODES_PER_SECTOR);
        *temp = inode;

        rw_sectors(1, (superblock.first_inode_block * SECTORS_PER_BLOCK) +
                        (inode.i_number / INODES_PER_SECTOR), (uint32_t)sector_buffer, WRITE);
}

// basically stolen from queso fuego, because my original idea was very inefficient (I was going 1 bit at a time from the get-go, similar to my PMM..)
uint32_t find_free_bit(uint32_t start_block, uint32_t length_blocks) {
        for(uint32_t i = 0; i < length_blocks; i++) {
                rw_sectors(SECTORS_PER_BLOCK, (start_block + i) * SECTORS_PER_BLOCK, (uint32_t)block_buffer, READ);

                uint32_t* temp = (uint32_t*)block_buffer;
                for(uint32_t j = 0; j < FS_BLOCK / sizeof(uint32_t); j++) {
                        if(temp[j] != 0xFFFFFFFF) {
                                for(uint32_t k = 0; k < 32; k++) {
                                        if(!(temp[j] & (1 << k))) {
                                                return (j * 32) + k;
                                        }
                                }
                        }
                }
        }
        return 0;
}

// FOR FUTURE STUFF
// DISCLAIMER: This create_file is far from complete, and is in no way a good create_file function. For one, I have
// hardly any error checking, two, I assume that our new entry will fit into the direct pointers, and I do not make
// use of single pointers, and three I do not update the superblocks first_free_data_bit and first_free_inode_bit right now
inode_t create_file(char* path) {

        // for the USER in particular, we will assume that they (as in me) are smart and don't create files with
        // the names ".", "..", and "/", as those are reserved, (UNLESS I am making a new directory).

        inode_t parent_inode = get_parent_inode(path);

        // make new inode
        inode_t new_inode = {0};
        new_inode.i_number = superblock.first_free_inode_bit;
        set_inode_bitmap(new_inode.i_number, true);

        superblock.first_free_inode_bit = find_free_bit(superblock.first_inode_bitmap_block,
                superblock.num_inodes_bitmap);
        
        new_inode.direct_pointers[0] = superblock.first_data_block + superblock.first_free_data_bit;
        set_data_bitmap(new_inode.direct_pointers[0], true);
        
        superblock.first_free_data_bit = find_free_bit(superblock.first_data_bitmap_block,
                superblock.num_data_bitmap);
        
        new_inode.file_type = FILE_TYPE_FILE;
        //TODO: also set the time created for the new inode
        new_inode.size = FS_BLOCK;

        update_inode(new_inode);

        // next up is linking up the file
        dir_entry_t new_entry = {0};
        new_entry.i_number = new_inode.i_number;
        strncpy(new_entry.name, get_name_path(path), sizeof(new_entry.name));

        uint32_t direct_blocks_to_read = bytes_to_blocks(parent_inode.size);
        uint32_t new_direct_blocks_to_read = bytes_to_blocks(parent_inode.size + sizeof(dir_entry_t));
        if(new_direct_blocks_to_read > direct_blocks_to_read) {
                // if it seems as though the directory will take up another block of data from adding
                // new_entry, then we need to initialize the next direct pointer (assuming there is one)

                if(direct_blocks_to_read >= 2) {
                        // use indirect pointers, as we have run out of direct blocks to use (oops!)
                }

                else {
                        parent_inode.direct_pointers[new_direct_blocks_to_read] = superblock.first_data_block +
                                superblock.first_free_data_bit;
                        superblock.first_free_data_bit = find_free_bit(superblock.first_data_bitmap_block,
                                        superblock.num_data_bitmap);

                        rw_sectors(SECTORS_PER_BLOCK,
                                        parent_inode.direct_pointers[new_direct_blocks_to_read]*SECTORS_PER_BLOCK,
                                       (uint32_t)block_buffer, READ);
                        memset(block_buffer, 0, sizeof(block_buffer));

                        rw_sectors(SECTORS_PER_bLOCK,
                                        parent_inode.direct_pointers[new_direct_blocks_to_read]*SECTORS_PER_BLOCK,
                                        (uint32_t)block_buffer, WRITE);
                }

        }
        
        bool done = false;
        for(uint32_t i = 0; i < direct_blocks_to_read; i++) {
                rw_sectors(SECTORS_PER_BLOCK, i*SECTORS_PER_BLOCK, (uint32_t)block_buffer, READ);

                dir_entry = (dir_entry_t*)block_buffer;
                for(uint32_t j = 0; j < DIR_ENTRIES_PER_BLOCK; j++, dir_entry++) {
                        if(dir_entry->i_number == 0) {
                                *dir_entry = new_entry;
                                done = true;
                                rw_sectors(SECTORS_PER_BLOCK, i*SECTORS_PER_BLOCK, (uint32_t)block_buffer, WRITE);
                                break;
                        }
                }
                if(done) {
                        break;
                }
        }

        parent_inode.size += sizeof(dir_entry_t);
        update_inode(parent_inode);

        if(current_dir_inode.i_number == parent_inode.i_number) {
                current_dir_inode = parent_inode;
        }

        if(root_inode.i_number == parent_inode.i_number) {
                root_inode = parent_inode;
        }

        update_superblock();
        return new_inode;
}

// might be the next one I make, as I NEED proper testing once I mount my fs 
bool print_dir(char* path) {
        uint32_t index = strrchr(path, '/') - path;
        char* temp = path;
        temp[index] = '\0';

        inode_t dir_inode = get_inode(temp);
        if(dir_inode.file_type != FILE_TYPE_DIR) {
                return false;
        }

        uint32_t total_entries = dir_inode.size / sizeof(dir_entry_t);
        uint32_t num_entries = 0;
        dir_entry_t* dir_entry = 0;

        uint32_t total_blocks_to_read = bytes_to_blocks(dir_inode.size);
        for(uint32_t i = 0; i < total_blocks_to_read; i++) {
                rw_sectors(SECTORS_PER_BLOCK, dir_inode.direct_pointers[i]*SECTORS_PER_BLOCK, (uint32_t)block_buffer,
                                READ);

                dir_entry = (dir_entry_t*)block_buffer;
                for(uint32_t j = 0; num_entries < total_entries && j < DIR_ENTRIES_PER_BLOCK; j++, dir_entry++) {

                        if(dir_entry->i_number != 0) {
                                num_entries++;

                                rw_sectors(1, (superblock.first_inode_block * SECTORS_PER_BLOCK) +
                                                (dir_entry->i_number / INODES_PER_SECTOR),
                                                (uint32_t)sector_buffer, READ);

                                inode_t* inode = (inode_t*)sector_buffer + (dir_entry->i_number % INODES_PER_SECTOR);

                                //kprintf the stuff out

                                if(inode->file_type == FILE_TYPE_FILE) {
                                        kprintf("\r\n%s        %s     ", dir_entry->name, "[FILE]");
                                }
                                else {
                                        kprintf("\r\n%s        %s     ", dir_entry->name, "[DIR]");
                                }

                        }
                }
        }
        return true;
}


// 
// The above functions are all the functions I will have available for RIGHT NOW. I've coded in, I think, all the necessary functions. So now, I have to first 
// mount the file system, and ensure that the functions I made above work, before I can move onto making the other functions, like the ones below.
//
inode_t create_dir(char* path) {
        return (inode_t){0};
}

bool delete_file(char* path) {
        return false;
}

bool delete_dir(char* path) {
        return false;
}

// FOR FUTURE FUNCTIONS, POSSIBLY:
// rename directory, rename file, move_to_directory (like a cd command), 
