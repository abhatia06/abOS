BITS 32

%MACRO isr_err_stub 1
isr_stub_%+%1:
    PUSH DWORD %1
    CALL exception_handler_error
    ADD ESP, 4    ; reset stack pointer
    IRET		; according to OSDev wiki, to return from an interrupt handler, IRET must be used, not RET.
%ENDMACRO

%MACRO isr_no_err_stub 1
isr_stub_%+%1:
    PUSH DWORD %1
    CALL exception_handler
    ADD ESP, 4
    IRET
%ENDMACRO

; 32 exception handlers:
EXTERN exception_handler
EXTERN exception_handler_error
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

; global stub table
GLOBAL isr_stub_table
isr_stub_table:
%ASSIGN i 0
%REP	32
    DD isr_stub_%+i
%ASSIGN i i+1
%ENDREP
