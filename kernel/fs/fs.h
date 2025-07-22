#pragma once

#include "stdint.h"
#define FS_BLOCK        4096
#define FS_SECTOR       512

#define file_type_file 0
#define file_type_dir 1

#define inodeStartAddr 3072     // 3KiB. We're going to be assuming superblock, and the two bitmaps = 1KiB each

// MACRO FUNCTIONS FOR CALCULATING STUFF
#define inode_block(i_number) ((i-number * sizeof(struct inode) / FS_BLOCK);    // finds which i-block your inode is in
#define inode_sector(i_number) (((inode_block(i_number) * FS_BLOCK) + inodeStartAddr) / FS_SECTOR)

extern inode_t current_dir_inode;
extern superblock_t superblock;
extern int block_buffer[FS_BLOCK];
extern int sector_buffer[FS_SECTOR];
extern inode_t root_inode;

typedef struct boot_record {
        uint8_t sector[8][FS_SECTOR];           // 8 sectors, each of 512 bytes, hence 2D array
} boot_record_t;

typedef struct inode {
        uint32_t i_number;                      // reserve inode 0, by the way
        uint32_t file_type;
        uint32_t size;                        // assume to be bytes
        fs_time_t time_accessed;
        fs_time_t time_modified;
        fs_time_t time_created;
        int direct_pointers[3];            // 3 direct pointers, index 0 is going to be the beginning one
        uint32_t single_indirect_block;         // 1 single indirect pointer
        uint8_t padding;
} inode_t;

typedef struct super_block {
        uint32_t size_of_data;
        uint32_t num_inodes;
        uint32_t num_inodes_bitmap;
        uint32_t num_inodes_per_block;
        uint32_t num_inodes_per_sector;
        uint32_t inode_size;            // likely going to be 128 bytes?
        uint32_t num_datablocks;
        uint32_t num_datablocks_bitmap;
        uint32_t starting_inode_addr;
        uint32_t max_file_size;
        uint32_t root_i_number;         // using inode_block() and inode_sector() functions can give us the root inode
        uint8_t padding
        uint32_t first_free_inode_bit;  // stole this off queso fuego (originally was gonna make a separate function)
        uint32_t first_free_data_bit;
} super_block_t;                // CURRENT SIZE: 41 BYTES + however big root_inode is

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
        char name[20];
} dir_entry_t;

typedef struct time {
        uint8_t second;
        uint8_t minute;
        uint8_t hour;
        uint8_t day;
        uint8_t month;
        uint32_t year;
} fs_time_t;

void rw_sectors(uint32_t sectors, uint32_t starting_sector, uint32_t address, int readwrite);
