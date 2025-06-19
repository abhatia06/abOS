#include "../stdint.h"
#include "idt.h"
#include <stdbool.h>

static idt_entry_t idt[256];
static idtr_t idtr;

static bool vectors[256];
extern void* isr_stub_table[];

/*
 * uint16_t base_low;      // lower 16 bits of ISR's address
 * uint16_t segsel;        // GDT segment selector that the CPU will load into CS before calling ISR
 * uint8_t donottouch;     // this is the reserved section, we can't use it, so it's always 0
 * uint8_t attributes;     // various flags, similar to GDT, like the permissions (0 for kernel), etc.
 * uint16_t base_high;     // higher 16 bits of ISR's address
 */

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
	idt_entry_t* descriptor = &idt[vector];

	descriptor->base_low	= (uint32_t)isr & 0xFFFF;
	descriptor->segsel	= 0x08;		// CS selector in GDT. Check boot.s to see it, (it's 0x08)
	descriptor->donottouch  = 0;
	descriptor->attributes  = flags;
	descriptor->base_high	= (uint32_t)isr >> 16;
}

void initIDT() {
	idtr.base = (uintptr_t)&idt[0];
	idtr.limit = (uint16_t)sizeof(idt_entry_t) * 256 - 1;

	for(uint8_t vector = 0; vector < 32; vector++) {
		idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
		vectors[vector] = true;
	}

	__asm__ volatile ("lidt %0" : : "m"(idtr));
	__asm__ volatile ("sti");
}

void exception_handler() {
	__asm__ volatile ("cli; hlt");
}

