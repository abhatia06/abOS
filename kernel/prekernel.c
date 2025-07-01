#include "memory/physical_memory_manager.h"
#include "memory/virtual_memory_manager.h"
#include "util/string.h"
#include "stdint.h"
#include "global_addresses.h"
#include "printlite.h"

__attribute__ ((section ("prekernel_entry"))) void prekernel_main() {
        // For some reason, the prekernel setup is INCREDIBLY slow. I don't know why, I am guessing it's due to the SMAP stuff?

        printS("Successfully entered Protected Mode!\n");
        printS("Successfully entered Lower-Memory Kernel!\n");
        printS("Initializing Physical Memory Manager...\n");
        initialize_pmm();

        typedef struct SMAP_entry {
                uint64_t base_address;
                uint64_t length;
                uint32_t type;
                uint32_t acpi;
        }__attribute__((packed)) SMAP_entry_t;

        uint32_t num_entries = *(uint32_t*)0x9000;
        SMAP_entry_t* entry = (SMAP_entry_t*)0x9004;
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

        deinit_memory_region(0x100000, 81920);  // Deinitialize some of the physical memory at 1MB, because that is for kernel
        deinit_memory_region(0x7EE0000, 20480); // Same with 0x7EE0000, but that is where the memory map lives

        printS("Initialized Physical Memory Manager!\n");
        *(uint32_t *)CURRENT_PAGE_DIR_ADDRESS = (uint32_t)directory;    // Save the current directory we're in

        printS("Initializing Virtual Memory Manager & Enabling Paging...\n");
        initialize_vmm();       // Initialize VMM

        printS("Initialized Virtual Memory Manager!\n");
        printS("Enabled Paging!\n");

        /*
        for(uint32_t virt = 0x100000; virt < 0x400000; virt += PAGE_SIZE) {
                free_page((void *)virt);
        }
        */      // I wanted to unmap the kernel from the virtual address, but it didn't work, sooo.. Whatever, who cares, not a big issue rn


        *(uint32_t*)TOTAL_MEMORY = total_memory;
        *(uint32_t*)USED_BLOCKS = used_blocks;
        *(uint32_t*)MAX_BLOCKS = max_blocks;
        // Reload CR3 register to flush TLB to update unmapped pages (literally move it to ECX, then back to CR3)
        __asm__ __volatile__ ("movl %%cr3, %%ecx; movl %%ecx, %%cr3" ::: "ecx");

        printS("Pre-kernel setup done! Getting ready to jump to virtual address 0xC0000000\n");
        clear();
        // Jump to 0xC0000000 (which is mapped to 0x100000). (void (*)(void)) is a function pointer cast (void ret type & void args)
        ((void (*)(void))0xC0000000)();


}
