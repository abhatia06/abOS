#include "../x86.h"
#include "pic.h"

void PIC_sendEOI(uint8_t irq) {
        if(irq >= 8) {
                outb(PIC2_COMMAND, PIC_EOI);                                                                                    }

        outb(PIC1_COMMAND, PIC_EOI);
}

void PIC_remap(int offset1, int offset2) {
	// This will send ICW1 saying we'll follow with ICW4 later on
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);	// starts initialization
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	
	// Send ICW2 with IRQ remapping
	outb(PIC1_DATA, offset1);
	outb(PIC2_DATA, offset2);	// This is supposed to be offset1+8, so I just have to remember that.
	
	outb(PIC1_DATA, 4);
	outb(PIC2_DATA, 2);

	outb(PIC1_DATA, ICW4_8086);
	outb(PIC2_DATA, ICW4_8086);

	// unmask both PICs. We're done now basically
	outb(PIC1_DATA, 0);	
	outb(PIC2_DATA, 0);	
}

//  Disables interrupts 
void pic_disable() {
	outb(PIC1_DATA, 0xFF);
	outb(PIC2_DATA, 0xFF);
}
