ENTRY(_start)

SECTIONS {
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
