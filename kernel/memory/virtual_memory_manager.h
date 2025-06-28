#pragma once

#include "../stdint.h"
#include <stdbool.h>

// For the VMM, I decided to follow the broken thorn tutorial and the OSDev wiki 
#define PAGE_SIZE 4096		// It is common practice to have each page be 4096 bytes. (4KB)
#define PAGE_TABLE_SIZE 1024	// According to OSDev wiki, each page table will hold 1024 entries
#define PAGE_DIRECTORY_SIZE 1024	// The virtual address format uses three sections: entry in a page directory
					// table, the page table index, and the offset into that page. Both the
					// PDT and PTs are arrays size 1024 according to brokenthorn. More specifically,
					// imagine PDT pointing to PTs, and PTs pointing to pages, (OSDev has a nice
					// image to understand this).

// The format is like so: 
// AAAAAAAAAA BBBBBBBBBB CCCCCCCCCCCC
// PDE        PTE        offset into page
#define PD_INDEX(address) (((address) >> 22) & 0x3FF) 	// bits 22-31 specify index of the PD entry
#define PT_INDEX(virtual_address) (((virtual_address) >> 12) & 0x3FF)
#define GET_FRAME(dir_entry) ((*dir_entry) & ~0xFFF) // 1111111111111111111000000000000. We don't want those 
							     // lower 12 bits, just the top ones. If you look at the
							     // enums below, 0x7FFFF000 is the huge binary I just 
							     // typed up there

// Broken thorn also has these, but they just use separate functions. Queso Fuego uses macros, so I decided to do
// the same.
#define SET_ATTRIBUTE(entry, attr) (*entry |= attr) 
#define CLEAR_ATTRIBUTE(entry, attr) (*entry &= ~attr)

// Clear old frame address, and then put in the new frame addresss (making sure to NOT touch the flags/attributes)
#define SET_FRAME(entry, address) (*entry = (*entry & ~0x7FFFF000 | (address & 0x7FFFF000)) 

// PDE and PT structures. These are ripped off of brokenthorn
typedef enum {
	PTE_PRESENT	= 0x01,
	PTE_WRITABLE	= 0x02,
	PTE_USER	= 0x04,
	PTE_WRITETHROUGH = 0x08,
        PTE_NOT_CACHEABLE = 0x10,
	PTE_ACCESSED	= 0x20,
	PTE_DIRTY	= 0x40,
	PTE_PAT		= 0x80,
	PTE_CPU_GLOBAL	= 0x100,
	PTE_LV4_GLOBAL	= 0x200,
	PTE_FRAME	= 0x7FFFF000,	
} PT_FLAGS;

typedef enum {
	PDE_PRESENT	= 0x01,
	PDE_WRITABLE	= 0x02,
	PDE_USER	= 0x04,
	PDE_PWT		= 0x08,
	PDE_PCD		= 0x10,
	PDE_ACCESSED	= 0x20,
	PDE_DIRTY	= 0x40,
	PDE_4MB		= 0x80,	// 0 = 4KB page, 1 = 4MB page
	PDE_CPU_GLOBAL	= 0x100, 
	PDE_LV4_GLOBAL	= 0x200,
	PDE_FRAME	= 0x7FFFF000,	
} PDT_FLAGS;

typedef uint32_t pt_entry;	// Page table entry
typedef uint32_t pd_entry;	// page directory entry
typedef uint32_t physical_address;
typedef uint32_t virtual_address;

/*	This is a separate option that OSDev wiki uses, but I decided to stick with brokenthorns, and used their idea
uint32_t page_directory[1024]__attribute__((aligned(4096)));
uint32_t page_table[1024]__attribute__((aligned(4096)));
*/

// Handle 4MB each, 1024 entries * 4096
typedef struct {
	pt_entry m_entries[PAGE_TABLE_SIZE];
} ptable __attribute__((aligned(4096)));

// Handle 4GB, 1024 entries * 4MB (because each directory points to a table)
typedef struct {
	pd_entry m_entries[PAGE_DIRECTORY_SIZE];
} pdirectory __attribute__((aligned(4096)));

extern ptable* table;
extern pdirectory* directory;
extern physical_address current_pd_address;

// If we used the OSDev version, to access an index we would just have to do uint32_t entry = page_table[index];
// for the second option, it's more blehhh, we have to do: pt_entry entry = table->m_entries[index];
// And if we want the pointers, we just have to do uint32_t* entry = &page_table[index];, and
// pt_entry* entry = &table->m_entries[index];


bool alloc_page(pt_entry* e);
void free_page(pt_entry* e);
pt_entry* get_pt_entry(ptable *pt, virtual_address address);
pd_entry* get_pd_entry(pdirectory *pd, virtual_address address);
bool set_pd(pdirectory* dir);
void flub_tlb_entry(virtual_address address);
bool map_page(void* phys_adress, void* virt_address);
void initialize_vmm();
