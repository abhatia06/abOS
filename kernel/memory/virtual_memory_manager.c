#include "../stdint.h"
#include "virtual_memory_manager.h"
#include <stdbool.h>
#include "physical_memory_manager.h"
#include "../util/string.h"
#include "../global_addresses.h"

#define EnablePaging()              \
    asm volatile (                  \
        "mov %%cr0, %%eax\n\t"      \
        "or $0x80000000, %%eax\n\t" \
        "mov %%eax, %%cr0"          \
        ::: "eax")

pdirectory* directory = 0;

bool alloc_page(pt_entry* e) {
        void* p = (void*)allocate_blocks(1);
        if(!p) {
                return 0;
        }

        SET_FRAME(e, (physical_address)p);
        SET_ATTRIBUTE(e, PTE_PRESENT);
        return 1;
}

void free_page(pt_entry* e) {
        void* p = (void*)GET_FRAME(e);
        if(p) {
                free_blocks((uint32_t)p, 1);
        }
        CLEAR_ATTRIBUTE(e, PTE_PRESENT);
}

void unmap_page(void* virt_address) {
        pt_entry* page = get_page((uint32_t)virt_address);

        SET_FRAME(page, 0);
        CLEAR_ATTRIBUTE(page, PTE_PRESENT);
}

pt_entry* get_pt_entry(ptable *pt, virtual_address address) {

        // If the pt exists, then we want to get the address to the entry that the virtual address corresponds to
        // in the page table.
        if(pt) {
                return &pt->m_entries[PT_INDEX(address)];
        }
        return 0;
}

pd_entry* get_pd_entry(pdirectory *pd, virtual_address address) {
        if(pd) {
                return &pd->m_entries[PD_INDEX(address)];
        }
        return 0;
}

// THIS IS STUFF I WILL ALSO PUT IN EXPLANATION.TXT
// Apparently, we can have a page directory for every process, and we simply have the kernel mapped in to every
// directory. Then, since we know that user mode can do processes via system calls that switch to kernel mode, then
// the kernel in every directory (not really multiple kernels, they all just point to the same kernel), can do system
// calls and it'll work just fine. Through this, we can give every process the illusion they have 4GB of free memory.
// This is useful, because we can then easily switch between processes by simply switching between a directory, which
// will house a different process, probably at 0x0 (because virtual addressing lets us put stuff at 0x0).
bool set_pd(pdirectory* dir) {
        if(!dir) {
                return false;
        }
        directory = dir;

        // might change to use EAX register cuz any GPR is kinda scaarrryyy
        __asm__ volatile ("movl %0, %%cr3" : : "r"(directory) : "memory" );

        return true;
}

void flush_tlb_entry(virtual_address address) {
        __asm__ volatile("cli; invlpg (%0); sti" : : "r"(address) : "memory" );
}

bool map_page(void* phys_address, void* virt_address) {
        pdirectory* pageDirectory = directory;

        pd_entry* e = &pageDirectory->m_entries[PD_INDEX((uint32_t)virt_address)];
        if( (*e & PTE_PRESENT) != PTE_PRESENT) {
                ptable* table = (ptable*)allocate_blocks(1);
                if(!table) {
                        return false;
                }

                map_page((void*)table, (void*)table);
                memset(table, 0, sizeof(ptable));

                pd_entry* entry = &pageDirectory->m_entries[PD_INDEX((uint32_t)virt_address)];

                SET_ATTRIBUTE(entry, PDE_PRESENT);
                SET_ATTRIBUTE(entry, PDE_WRITABLE);
                SET_ATTRIBUTE(entry, PDE_USER);	// THESE ARE JUST TEMPORARY. I AM SETTING IT TO BE ABLE TO BE ACCESSED BY RING 3 FOR TESTING PURPOSES
                SET_FRAME(entry, (physical_address)table);

        }
        ptable* table = (ptable*)GET_FRAME(e);

        pt_entry* page = &table->m_entries[PT_INDEX((uint32_t)virt_address)];
        SET_ATTRIBUTE(page, PTE_PRESENT);
        SET_ATTRIBUTE(page, PTE_WRITABLE);
        SET_ATTRIBUTE(page, PTE_USER);	// THESE ARE JUST TEMPORARY. I AM SETTING IT TO BE ABLE TO BE ACCESSED BY RING 3 FOR TESTING PURPOSES
	// In the future, though, I might actually have to make map_pages require a 3rd argument, (a 0 or 1), that dictates if we want this map page to be accessible
	// to the user. 
        SET_FRAME(page, (uint32_t)phys_address);
        return true;
}

void initialize_vmm() {
        ptable* table = (ptable*)allocate_blocks(1);
        if(!table) {
                //kprintf("We got to here. (NO TABLE?)");
                return;
        }

        ptable* table2 = (ptable*)allocate_blocks(1);
        if(!table2) {
                return;
        }

        memset(table, 0, sizeof(ptable));
        memset(table2, 0, sizeof(ptable));

        // Identity map the first 4MB of memory
        for(uint32_t i = 0, frame=0x0, virt=0x00000000; i<1024; i++, frame+=4096, virt+=4096) {
                pt_entry page = 0;
                SET_ATTRIBUTE(&page, PTE_PRESENT);
                SET_ATTRIBUTE(&page, PTE_WRITABLE);
                SET_FRAME(&page, frame);

                table2->m_entries[PT_INDEX(virt)] = page;
        }


        // Map kernel to 3GB+ addresses (going to be higher half of kernel)
        for(uint32_t i = 0, frame = 0x100000, virt=0xC0000000; i<1024; i++, frame+=4096, virt+=4096) {
                pt_entry page = 0;
                SET_ATTRIBUTE(&page, PTE_PRESENT);
                SET_ATTRIBUTE(&page, PTE_WRITABLE);
                SET_ATTRIBUTE(&page, PTE_USER);	// THESE ARE JUST TEMPORARY. I AM SETTING IT TO BE ABLE TO BE ACCESSED BY RING 3 FOR TESTING PURPOSES
                SET_FRAME(&page, frame);

                table->m_entries[PT_INDEX(virt)] = page;
        }

        pdirectory* dir = (pdirectory*)allocate_blocks(3);
        if(!dir) {
                return;
        }

        memset(dir, 0, sizeof(pdirectory));

        pd_entry* entry = &dir->m_entries[PD_INDEX(0xC0000000)];
        SET_ATTRIBUTE(entry, PDE_PRESENT);
        SET_ATTRIBUTE(entry, PDE_WRITABLE);
        SET_ATTRIBUTE(entry, PDE_USER);	// THESE ARE JUST TEMPORARY. I AM SETTING IT TO BE ABLE TO BE ACCESSED BY RING 3 FOR TESTING PURPOSES
        SET_FRAME(entry, (physical_address)table);

        pd_entry* entry2 = &dir->m_entries[PD_INDEX(0x00000000)];
        SET_ATTRIBUTE(entry2, PDE_PRESENT);
        SET_ATTRIBUTE(entry2, PDE_WRITABLE);
        SET_FRAME(entry2, (physical_address)table2);

        set_pd(dir);

        EnablePaging();
}

pt_entry *get_page(virtual_address address) {

        // Get page directory
        pdirectory *pd = directory;

        // Get page table in directory
        pd_entry   *entry = &pd->m_entries[PD_INDEX(address)];
        ptable *table = (ptable *)GET_FRAME(entry);

        // Get page in table
        pt_entry *page = &table->m_entries[PT_INDEX(address)];

        // Return page
        return page;
}
