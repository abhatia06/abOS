BITS 32

GLOBAL _start
EXTERN main

SECTION .text

_start:

    MOV ESP, stack_end     ; This sets up the stack.
    ; I ended up learning about external calls and whatnot through THIS video below:
    ; https://www.youtube.com/watch?v=jDZuTpCE8l8
    CALL main
hang:
    JMP hang

    ; everything below this doesnt get used, but I dont wanna delete it all just yet lol 
    MOV EDI, 0xB8000
    MOV AH, 0x0F
    MOV ESI, message

.print_loop:
    LODSB
    CMP AL, 0
    JE .done
    STOSW
    JMP .print_loop

.done:
    HLT
    JMP $

message: db 'Hello World from Kernel!', 0

SECTION .bss

stack_begin:
    RESB 8192
stack_end:
