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
        uint32_t direct_pointers[6];            // 4 direct pointers, mostly because the kernel needs 4 blocks (will likely change to 5 soon)
        uint32_t single_indirect_block;         // 1 single indirect pointer
        uint8_t padding[3];
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
} __attribute__((packed)) superblock_t;         // 128 bytes. Hopefully. I hope I don't suck at my 4's multiplication table.

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
} __attribute__((packed)) dir_entry_t;        // 64 bytes

//TODO: make the inode struct take up a power of 2 space 

#define INODES_PER_SECTOR (FS_SECTOR/sizeof(inode_t))
#define INODES_PER_BLOCK (FS_BLOCK/sizeof(inode_t))
#define DIR_ENTRIES_PER_BLOCK (FS_BLOCK/sizeof(dir_entry_t))
