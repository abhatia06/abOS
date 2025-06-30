#include "stdint.h"
#include "stdio.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "interrupts/exceptions.h"
#include "memory/physical_memory_manager.h"
#include "util/string.h"
#include "memory/virtual_memory_manager.h"

void kernel_main() { 
	// For right now, everything is messed up. We need to re-initialize the stack, and re-clear-out the bss section. But, this green K is a good way to know that
	// we are now in higher kernel, and our VMM works.
	volatile int *ptr = (int*)0xB8000;
	*ptr = 0x2F4B;
	while(1);
	/*
        kprintf("Higher kernel loaded!");
	while(true) { 
		char* command = readline();
                if(strcmp((const char*)command, "printmem") == 0) {
                        kprintf("\r\n");
                        print_memmap_command();
                        kprintf("\r\n");
                }
        }
	*/

}
