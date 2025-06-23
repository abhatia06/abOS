#pragma once

// This is all based off of OSDev's Interrups_Tutorial page. 
typedef struct {
	uint16_t base_low;	// lower 16 bits of ISR's address
	uint16_t segsel;	// GDT segment selector that the CPU will load into CS before calling ISR
	uint8_t donottouch;	// this is the reserved section, we can't use it, so it's always 0
	uint8_t attributes;	// various flags, similar to GDT, like the permissions (0 for kernel), etc.
	uint16_t base_high;	// higher 16 bits of ISR's address
}__attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t limit;
	uint32_t base;
}__attribute__((packed)) idtr_t;

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
void initIDT(void);

__attribute__((noreturn)) void exception_handler(uint32_t vector_number);
__attribute__((noreturn)) void exception_handler_error(uint32_t vector_number, uint32_t error_code);

typdef struct {
	uint32_t eflags;
	uint32_t cs;
	uint32_t eip;
} __attribute__((packed)) int_frame_32_t;
