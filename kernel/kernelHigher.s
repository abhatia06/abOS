BITS 32

GLOBAL kernel_start
EXTERN kernel_main
EXTERN bss_begin
EXTERN bss_final

;bss_size equ bss_end - bss_start

SECTION .text

kernel_start:                 ; Supposedly we use _ before a function to indicate it is a CDECL calling convention.

    ;mov edi, bss_begin     ; ← USE ADDRESS, not dereference
    ;mov ecx, bss_final
    ;sub ecx, edi
    ;xor eax, eax
    ;rep stosb              ; STOSB for safety with odd sizes

    MOV EDI, bss_begin
    MOV EAX, bss_final
    SUB EAX, EDI
    SHR EAX, 2
    MOV ECX, EAX
    XOR EAX, EAX
    REP STOSD

    MOV ESP, stack_end
    CALL kernel_main
    JMP hang

hang:
    JMP hang


SECTION .bss

stack_begin:
    RESB 4096
stack_end:
