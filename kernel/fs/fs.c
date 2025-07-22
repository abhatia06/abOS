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

