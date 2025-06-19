#pragma once

// move PIC, because in protected mode, its IRQs will conflict with CPU (intel reserves up to 0x1F)
#define PIC1	0x20
#define PIC2	0xA0
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI 	0x20

void PIC_sendEOI(uint8_t irq);

// initialization
#define ICW1_ICW4	0x01	// indicates ICW4 will be present. Lets us define stuff on ICW4
#define ICW1_SINGLE	0x02	// single (cascade) mode
#define ICW1_INTERVAL4	0x04	// call address interval 4
#define ICW1_LEVEL	0x08	// level triggered mode
#define ICW1_INIT	0x10	// the ACTUAL initialization

#define ICW4_8086	0x01	// 8086 mode
#define ICW4_AUTO	0x02	// auto EOI
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM	0x10

void PIC_remap(int offset1, int offset2);
void pic_disable(void);
