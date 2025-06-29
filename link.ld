ENTRY(_start)

SECTIONS {
	. = 0x50000; /* higher kernel will be at physical address 0x100000, (mapped to VA 0xC0000000), so we load this 3rd-stage bootloader to 0x50000 instead.
	.text : {
		*(.text*)
	}
	.rodata : {
		*(.rodata)
		*(.rodata*)
	}
	.data : {
		*(.data)
	}
	.bss : {
		bss_start = .;
		*(.bss)
		*(COMMON)
		bss_end = .;
	}
}
