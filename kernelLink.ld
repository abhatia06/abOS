ENTRY(prekernel_main)

SECTIONS {
        . = 0x50000;
        .text : {
            *(prekernel_entry);
            *(.text)
            *(.text*)
        }
        .rodata : {
            *(.rodata)
            *(.rodata*)
        }
        .data : {
            *(.data)
            *(.data*)
        }
        .bss : {
            bss_begin = .;
            *(.bss)
            *(COMMON)
            bss_final = .;
        }
}
