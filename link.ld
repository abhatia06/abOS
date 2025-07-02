ENTRY(_start)

SECTIONS {
        . = 0xC0000000;
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
