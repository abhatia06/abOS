#include "memory/physical_memory_manager.h"
#include "memory/virtual_memory_manager.h"
#include "util/string.h"
#include "stdint.h"

void main() {
	initialize_pmm();

	typedef struct SMAP_entry {
        	uint64_t base_address;
        	uint64_t length;
        	uint32_t type;
        	uint32_t acpi;
	}__attribute__((packed)) SMAP_entry_t;

	uint32_t num_entries = *(uint32_t*)0x9000;
	SMAP_entry_t* entry = (SMAP_entry_t*)0x9004;

	for(uint32_t i = 0; i < num_entries; i++, entry++) {
		if(entry->type ==1) {
			initialize_memory_region((uint32_t)entry->base_address, (uint32_t)entry->length);
		}
	}
	deinit_memory_region(0x100000, 81920);

	initialize_vmm();


	*(uint32_t *)CURRENT_PAGE_DIR_ADDRESS = (uint32_t)current_page_directory;

	((void (*)(void))0xC0000000)();


}
