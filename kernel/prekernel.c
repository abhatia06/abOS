#include "memory/physical_memory_manager.h"
#include "memory/virtual_memory_manager.h"
#include "util/string.h"
#include "stdint.h"
#include "global_addresses.h"
#include "printlite.h"


__attribute__ ((section ("prekernel_entry"))) void prekernel_main() {
        // For some reason, the prekernel setup is INCREDIBLY slow. I don't know why, I am guessing it's due to the SMAP stuff?

        asm volatile ("ltr %0" :: "r"(0x28));
        printS("Successfully entered Protected Mode!\n");
        printS("Successfully entered Lower-Memory Kernel!\n");
        printS("Initializing Physical Memory Manager...\n");
        initialize_pmm();        //TODO: initializing pmm takes like 10-15 seconds. Fix unoptimization in PMM

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

        // These were part of the issue I was having. Apparently, since we weren't de-initializing the regions that house our OS stuff among other things, it was 
        // causing issues? I don't fully know WHY it was causing issues, since, y'know, the PMM and the VMM don't actually load anything onto the addresses that are
        // "mapped" or "allocated", (we still haven't made a malloc yet), but alright, I guess. I need to do more research to figure out why this was an issue.
        deinit_memory_region(0x0, 0x12000);
        deinit_memory_region(MEMMAP_AREA, max_blocks/BLOCK_SIZE);
        deinit_memory_region(0x100000, 81920);  // Deinitialize some of the physical memory at 1MB, because that is for kernel

        printS("Initialized Physical Memory Manager!\n");

        printS("Initializing Virtual Memory Manager & Enabling Paging...\n");
        initialize_vmm();       // Initialize VMM

        printS("Initialized Virtual Memory Manager!\n");
        printS("Enabled Paging!\n");

        // managed to get this to work
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
        printS("Pre-kernel setup done! Getting ready to jump to virtual address 0xC0000000\n");

        //map_page((void*)0x200000, (void*)0xBFFFF000);
        //map_page((void*)0x500000, (void*)0x500000);        //these are for testing. Will remove eventually
        clear();
        // Jump to 0xC0000000 (which is mapped to 0x100000). (void (*)(void)) is a function pointer cast (void ret type & void args)
        ((void (*)(void))0xC0000000)();


}
