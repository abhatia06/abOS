BITS 32

GLOBAL _start
EXTERN main

SECTION .text

_start:        ; supposedly we use _ before to indicate CDECL calling convention

    ; The code below zeroes out the BSS. Why do we need to do that? I believe this reddit page answers it pretty well:
    ; https://www.reddit.com/r/AskProgramming/comments/7r8scm/why_do_we_have_to_clear_bss_in_assembly/
    MOV EDI, bss_start    ; We mark where BSS is via the link loader. 
    MOV EAX, bss_end      ; and we obviously mark the end too via link loader
    XOR EAX, EAX          ; Now we zero out EAX
    REP STOSD             ; And we store every byte starting from [EDI] with zeroes (EAX)
    
    MOV ESP, stack_end     ; This sets up the stack.
    ; I ended up learning about external calls and whatnot through THIS video below:
    ; https://www.youtube.com/watch?v=jDZuTpCE8l8
    CALL main
hang:
    JMP hang

SECTION .bss

stack_begin:
    RESB 8192
stack_end:
