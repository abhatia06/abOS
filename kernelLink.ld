ENTRY(kernel_start)

SECTIONS {
        . = 0xC0000000;

        .text : {
                *(.text*)
        }
        .rodata ALIGN(4096) : {
                *(.rodata)
                *(.rodata*)
        }
        .data ALIGN(4096) : {
                *(.data)
        }
        .bss ALIGN(4096) : {
                bss_begin = .;
                *(.bss)
                *(COMMON)
                bss_final = .;
        }
}
