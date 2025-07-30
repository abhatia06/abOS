#pragma once

#include "stdint.h"
#define FS_BLOCK        4096
#define FS_SECTOR       512
#define SECTORS_PER_BLOCK 8
#define SUPERBLOCK_DISK_SECTOR 8        // first 8 sectors are for superblock

#define file_type_file 0
#define file_type_dir 1

// MACRO FUNCTIONS FOR CALCULATING SECTION & BLOCK OF INODE GIVEN ITS I_NUMBER
#define inode_block(i_number) ((i_number * sizeof(struct inode)) / FS_BLOCK)    // finds which i-block your inode is in
#define inode_sector(i_number, superblock) (((inode_block(i_number) * FS_BLOCK) + (superblock.first_inode_block*FS_BLOCK)) / FS_SECTOR)

typedef struct boot_record {
        uint8_t sector[8][FS_SECTOR];           // 8 sectors, each of 512 bytes, hence 2D array
} boot_record_t;

typedef struct time {
        uint8_t second;
        uint8_t minute;
        uint8_t hour;
        uint8_t day;
        uint8_t month;
        uint16_t year;
        uint8_t padding;
} __attribute__((packed)) fs_time_t;        // 8 bytes

typedef struct inode {
        uint32_t i_number;                      // reserve inode 0, by the way
        uint8_t file_type;
        uint32_t size;                        // assume to be bytes
        fs_time_t time_accessed;
        fs_time_t time_modified;
        fs_time_t time_created;
        uint32_t direct_pointers[4];            // 4 direct pointers, mostly because the kernel needs 4 blocks (will likely change to 5 soon)
        uint32_t single_indirect_block;         // 1 single indirect pointer
        uint8_t padding[11];
} __attribute__((packed)) inode_t;        // 64 bytes

typedef struct super_block {
        //uint32_t size_of_data;
        uint32_t num_inodes;
        uint32_t num_inodes_bitmap;
        uint32_t num_data_bitmap;
        uint32_t num_inodes_per_block;
        uint32_t num_inodes_per_sector;
        uint32_t num_inode_blocks;
        uint32_t first_inode_bitmap;
        uint32_t first_inode_bitmap_block;
        uint32_t first_data_bitmap_block;
        uint32_t first_data_bitmap;
        uint32_t first_inode_block;
        uint32_t first_data_block;
        uint32_t inode_size;            // likely going to be 128 bytes?
        uint32_t num_datablocks;
        uint32_t max_file_size;
        uint32_t root_i_number;         // using inode_block() and inode_sector() functions can give us the root inode
        uint32_t first_free_inode_bit;  // stole this off queso fuego (originally was gonna make a separate function)
        uint32_t first_free_data_bit;
        uint32_t device_number;        // for a future device manager (https://wiki.osdev.org/Device_Management)
        uint8_t padding[52];
} superblock_t;         // 128 bytes. Hopefully. I hope I don't suck at my 4's multiplication table.

// everything below is for the files & directories

/*
 * According to the IBM website for directories (https://www.ibm.com/docs/bg/aix/7.2.0?topic=systems-directories), each directory entry contains a file or
 * subdirectory name, and an i-node number.
 * Each directory will house, at the very least, two things: one will be a directory entry called ".", this directory entry will simply be the entry
 * that contains the i-node for the directory itself. The second will be an entry called "..", which will be an entry that contains the i-node for
 * the parent directory, or the directory that houses the current ('.') directory.
 */
typedef struct dir_entry_t {
        // each direcotires entry will simply appear as an inode number that points to the actual inode that points to the actual data block
        // for the file in the directory
        uint32_t i_number;      
        char name[60];
} dir_entry_t;        // 64 bytes

//TODO: make the inode struct take up a power of 2 space 

#define INODES_PER_SECTOR (FS_SECTOR/sizeof(inode_t))
#define INODES_PER_BLOCK (FS_BLOCK/sizeof(inode_t))
#define DIR_ENTRIES_PER_BLOCK (FS_BLOCK/sizeof(dir_entry_t))

extern inode_t current_dir_inode;
extern superblock_t superblock;
extern int block_buffer[FS_BLOCK];
extern int sector_buffer[FS_SECTOR];
extern inode_t root_inode;

// this general helper function was only added later, hence why you might see the code in this function seemingly
// copy and pasted in other functions. I was just too lazy to edit those functions.
// Furthermore, this is the only function that is defined in the header file and not its C file. This is because format_disk.c uses this file, and 
// I don't want to have to link togetheer fs.c and format_disk.c, so I just decided to put this function here.
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

void rw_sectors(uint32_t sectors, uint32_t starting_sector, uint32_t address, int readwrite);
bool load_file(inode_t* inode, uint32_t address);
bool save_file(inode_t* node, uint32_t address);
inode_t get_inode_in_dir(inode_t current_dir, char* file);
inode_t get_inode(char* path);
inode_t get_parent_inode(char* path);
void set_inode_bitmap(uint32_t bit, bool set);
void set_data_bitmap(uint32_t bit, bool set);
char* get_name_path(char* path);
void update_superblock();
void update_inode(inode_t inode);
uint32_t find_free_bit(uint32_t start_block, uint32_t length_blocks);
inode_t create_file(char* filepath);
bool print_dir(char* path);
bool delete_file(inode_t* inode);
