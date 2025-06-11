; this is my first attempt of making a kernel. I realized from OSDev wiki that I needed to re-initialize the stack, because
; switching to protected mode may have messed up the segment pointers (SS, SP). So, I decided to have the kernel set up the
; stack. I don't know if this is actually correct, because I have yet to actually research kernels written in assembly.
; most tutorials seem to immediately set up the linker and kernel.c. I don't want to do that just yet.
BITS 32
ORG 0x100000

SECTION .text

set_up_stack:

    MOV ESP, stack_end

SECTION .bss

stack_begin:
    RESB 8192
stack_end:
