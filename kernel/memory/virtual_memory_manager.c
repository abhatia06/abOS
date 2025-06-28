#include "virtual_memory_manager.h"
#include "../stdint.h"
#include <stdbool.h>
#include "../stdio.c"
#include "physical_memory_manager.h"



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
	current_pd_address = dir;

	// might change to use EAX register cuz any GPR is kinda scaarrryyy
	__asm__ volatile ("movl %0, %%cr3" : : "r"(current_pd_address) : "memory" );

	return true;
}

void flush_tlb_entry(virtual_address address) {
	__asm__ volatile("cli; invlpg (%0); sti" : : "r"(address) : "memory" );
}


// UNFINISHED. WILL COME BACK TO IT TOMORROW, TOO TIRED TODAY. Didn't do a lot of work unfortunately today :(
/*
bool map_page(void* physical_address, void* virtual_address) {
	pdirectory* pageDirectory = get_directory();

	pd_entry* e = &pageDirectory->m_entries[PD_INDEX((uint32_t)virtual_address)];
	if( (*e & PTE_PRESENT) != PTE_PRESENT) {
		ptable* table = (ptable*)allocate_blocks(1);
		if(!table) {
			return;
		}

		
	}
}
*/

/*
pt_entry* get_page(virtual_address address) {
        //Get page directory
        pdirectory* pd = current_page_directory;

        //Get page table in directory
        pd_entry* entry = get_pd_entry(pd, address);
        page_table* table = (ptable*)PAGE_PHYS_ADDRESS;

        //Get page in table
        pt_entry* page = get_pt_entry(table, address);

        //Return page
        return page;
}
*/

