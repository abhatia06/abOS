#include "fs.h"
#include "../stdint.h"
#include "../stdio.h"
#include "../x86.h"

#define READ 0x20
#define WRITE 0x30

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

			// 400ns delay. (https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays), assume each I/O operation takes 30ns
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

// assume that inode is properly initialized, and its direct block pointers do in fact point to existing data blocks
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
                rw_sectors(inode->direct_pointers[i], inode->direct_pointers[0], address + offset, READ);
                offset+=4096;   // add 4KiB to offset, b/c each block is 4KiB
        }

        return true;
}

bool save_file(inode_t* node, uint32_t address) {
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
                rw_sectors(inode->direct_pointers[i], inode->direct_pointers[0], address + offset, WRITE);
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
        uint32_t file_size_bytes = current_dir->size;
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

                        if(strcmp(dir_entry->name, file) == 0) {	// TODO: implement strncmp just in case 
                                rw_sectors(1, inode_sector(dir_entry->i_number), (uint32_t)sector_buffer, READ);
				// an inode is <512 bytes, meaning 1 sector has multiple inodes (hopefully) packed together
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
        return (inode_t){0};
}

// ideas for other functions, perhaps

bool create_file(char* name, uint32_t size, uint32_t address) {
        return false;
}

bool delete_file(inode_t* inode) {
        return false;
}

