#include "stdint.h"
#include "stdio.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "interrupts/exceptions.h"
#include "memory/physical_memory_manager.h"

void memorysetup() {	// Right now the function is really only for testing purposes, but it does seem to work, so that's nice.
        initialize_pmm();
        initialize_memory_region(0x100000, 4096);
        int32_t checker = find_free_blocks(1);
        kprintf("bit address: %d\r\n", checker);
}

void main() {
	// I moved this up here because, somehow, interrupts were being fired as soon as the IDT was being initialized, leading to a double fault. So, I moved it up here
	pic_disable();	// temporarily stop listening to interrupts (mask all interrupts coming in). 
	memorysetup();
	print_memory_map();
	initialize_vmm();	// Initialize the VMM
	kprintf("\r\n");
	initIDT();	// Set up the IDT
	idt_set_descriptor(0, (uint32_t)div_by_0_handler, 0x8E);	// 0x8E is the interrupt gate flag. Others might use trap gate, but idc rn
	idt_set_descriptor(0x0E, (uint32_t)page_fault_handler, 0x8E); 	// page fault handler is interrupt 14, and it sends an error code 
	PIC_remap(0x20) // Look at the table, all external interrupts begin at 0x20. 

	idt_set_descriptor(0x20, PIT_handler, 0x8E);
	idt_set_descriptor(0x21, (uint32_t)keyboard_handler, 0x8E);	// We can finally have a keyboard handler and accept user input!!
	IRQ_clear_mask(0);	// We need to re-enable listening to interrupts from IRQ0, which is connected to the PIT channel 0
	IRQ_clear_mask(1);	// We need to re-enable listening to interrupts from IRQ1, which is connected to the keyboard device

	//Set default PIT timer IRQ rate to be about 1 millisecond (1193182hz/1193hz is roughly 1000 hz)
	set_PIT(0, 2, 1193)

	//uint8_t div0test = 0/0;	// This SHOULD throw an interrupt (NOTE FROM THE FUTURE: it does)
	
	const char* not_really_far_string = "far string";
	puts("Hello World from C!\n");
	kprintf("Formatted %% %c %s %ls\n" 'a', "string", not_really_far_string);
	kprintf("Formatted %d %i %x %p %o %hd %hi %hhu %hhd\n", 1234, -5678, 0xdead, 0xbeef, 012345, (short)27, (short)-42, (unsigned char)20, (signed char)-10);
	kprintf("Formatted %ld %lx %lld %llx\n", -100000000l, 0xdeadbeeful, 10200300400ll, 0xdeadbeeffeebdaedull);

	// This is a test to see if our VMM actually works:
        map_page((void*)0xB8000, (void*)0xC00B8000);
        volatile char* vid = (volatile char*)0xC00B8000;        // 0xB8000 should map to 0xC00B8000 now, so writing at
                                                                // 0xC00B00 should write to 0xB8000 instead. Inside the
                                                                // page table, (I believe it is entry 184, PD 768),
                                                                // bits 31-12 will be the address (0xB8000), with the
                                                                // rest of the bits being the flags (in this case, it
                                                                // will be PRESENT and WRITABLE). So, writing to
                                                                // 0xC00B8000 MUST write to 0xB8000.
        vid[0] = 'X';
        vid[1] = 0x07;
	
	kprintf("Type your name: ");
	char* name = readline();
	kprintf("Hello, %s!\n", name);

	//__asm__ volatile("iret");	This will ALSO throw an interrupt (interrupt 13, chcek the OSDev IDT to see what it does)
	return;
}
