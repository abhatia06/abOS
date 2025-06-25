#include "stdint.h"
#include "stdio.c"
#include "interrupts/idt.h"
#include "interrupts/exceptions.h"
#include "interrupts/pic.h"

void main() {
	
	initIDT();	// Set up the IDT
	idt_set_descriptor(0, (uint32_t)div_by_0_handler, 0x8E);	// 0x8E is the interrupt gate flag. Others might use trap gate, but idc rn
	pic_disable();	// temporarily stop listening to interrupts (mask all interrupts coming in)
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

	kprintf("Type your name: ");
	char* name = readline();
	kprintf("Hello, %s!\n", name);
	return;
}
