#include "memory/physical_memory_manager.h"
#include "memory/virtual_memory_manager.h"
#include "util/string.h"
#include "stdint.h"
#include "global_addresses.h"
#include "printlite.h"
#include "fs/fs.h"
#include "stdio.h"

__attribute__ ((section ("prekernel_entry"))) void prekernel_main() {
        // For some reason, the prekernel setup is INCREDIBLY slow. I don't know why, I am guessing it's due to the SMAP stuff?
        asm volatile ("ltr %0" :: "r"(0x28));
        kprintf("Successfully entered Protected Mode!\n");
        kprintf("Successfully entered Lower-Memory Kernel!\n");
        kprintf("Initializing Physical Memory Manager...\n");
        initialize_pmm();

        typedef struct SMAP_entry {
                uint64_t base_address;
                uint64_t length;
                uint32_t type;
                uint32_t acpi;
        }__attribute__((packed)) SMAP_entry_t;

        uint32_t num_entries = *(uint32_t*)0xA000;
        SMAP_entry_t* entry = (SMAP_entry_t*)0xA004;
        SMAP_entry_t* last_entry = &entry[num_entries - 1];

        for(uint32_t i = 0; i < num_entries; i++, entry++) {
                if(entry->type ==1) {
                        initialize_memory_region((uint32_t)entry->base_address, (uint32_t)entry->length);
                }
        }


        uint32_t total_memory = (uint32_t)(last_entry->base_address + last_entry->length-1);
        //*(uint32_t*)TOTAL_MEMORY = total_memory;
        //*(uint32_t*)USED_BLOCKS = used_blocks;
        //*(uint32_t*)MAX_BLOCKS = max_blocks;

        deinit_memory_region(0x0, 0x12000);
        deinit_memory_region(MEMMAP_AREA, max_blocks/BLOCK_SIZE);
        deinit_memory_region(0x100000, 81920);  // Deinitialize some of the physical memory at 1MB, because that is for kernel

        //printS("Loading the root directory...\n");

        //superblock = *(superblock_t*)0x8C00;
        //rw_sectors(1, superblock.first_inode_block*8, (uint32_t)sector_buffer, READ);
        //root_inode = *((inode_t*)sector_buffer + 1);
        //superblock.root_i_number = (uint32_t)&root_inode;
        //current_dir_inode = root_inode;

        kprintf("Initialized Physical Memory Manager!\n");
        *(uint32_t *)CURRENT_PAGE_DIR_ADDRESS = (uint32_t)directory;

        kprintf("Initializing Virtual Memory Manager & Enabling Paging...\n");
        initialize_vmm();       // Initialize VMM

        kprintf("Initialized Virtual Memory Manager!\n");
        kprintf("Enabled Paging!\n");


        for(uint32_t virt = 0x100000; virt < 0x400000; virt += PAGE_SIZE) {
                unmap_page((void *)virt);
        }

        *(uint32_t*)TOTAL_MEMORY = total_memory;
        *(uint32_t*)USED_BLOCKS = used_blocks;
        *(uint32_t*)MAX_BLOCKS = max_blocks;
        // Reload CR3 register to flush TLB to update unmapped pages (literally move it to ECX, then back to CR3)
        __asm__ __volatile__ ("movl %%cr3, %%ecx; movl %%ecx, %%cr3" ::: "ecx");

        *(uint32_t *)CURRENT_PAGE_DIR_ADDRESS = (uint32_t)directory;
        //*(uint32_t *)CURRENT_PD_ADD = (uint32_t)current_pd_address;   // already a uint32_t, but whatever
        kprintf("Pre-kernel setup done! Getting ready to jump to virtual address 0xC0000000\n");


        //map_page((void*)0x200000, (void*)0xBFFFF000);
        //map_page((void*)0x500000, (void*)0x500000);

        /*
        while(true) {
                __asm__ volatile("cli; hlt");
        }
        */

        clrscr();
        // Jump to 0xC0000000 (which is mapped to 0x100000). (void (*)(void)) is a function pointer cast (void ret type & void args)
        ((void (*)(void))0xC0000000)();
        __asm__ volatile("cli;hlt" : : "a"(0xbeefdead));
}
