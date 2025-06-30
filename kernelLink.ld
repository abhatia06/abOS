ENTRY(kernel_main)

SECTIONS {
        . = 0xC0000000;

        .text : ALIGN(0x1000) {
                *(.text*)
        }
        .rodata : ALIGN(0x1000) {
                *(.rodata)
                *(.rodata*)
        }
        .data : ALIGN(0x1000) {
                *(.data)
        }
        .bss : ALIGN(0x1000) {
                bss_start = .;
                *(.bss)
                *(COMMON)
                bss_end = .;
        }
}
