BITS 16
ORG 0x7C00	      ; this address is where the BIOS starts reading from during boot.

KERNEL_LOAD_SEG equ 0x1000
KERNEL_START_ADDR equ 0x100000

main:
    CLI
    MOV AX, 0x0		; We put 0x0 into AX because we cant directly put values into the segment registers
    MOV SS, AX		; SS:SP is a segment for the stack segment. We are trying to basically initialize our stack
    MOV SP, 0x7C00	; We initialize our stack at 0x7C00
    STI

    ; The code below follows the CHS reading format
    MOV BX, KERNEL_LOAD_SEG   ; https://stackoverflow.com/questions/67613202/bios-interrupt-13-read-sector-not-working
    MOV DH, 0x0		; We want to read from Head 0
    MOV DL, 0x80	; We want to read from  the first hard drive (0x80)
    MOV CL, 0x02	; We want to start from the 2nd sector (first sector is bootloader)
    MOV CH, 0x0		; We want to read from cylinder 0
    MOV AH, 0x02	; 0x02 means we want to read, 0x03 means we want to write
    MOV AL, 8		; The number of sectors we want to read (8). 
    INT 0x13		; The final piece of the puzzle. Interrupt 0x13 is for the BIOS Disk Service
    JC DISK_READ_ERROR  ; BIOS sets the carry flag if the call failed. JC jumps if carry flag is set.

enterProtectedMode:
    CLI
    LGDT [gdtrd]
    MOV EAX, CR0       ; We need to set the PE bit to 1 on the CR0 register to switch to protected mode, and we do it
    OR EAX, 1          ; through these three registers.
    MOV CR0, EAX
    JMP 0x08:PModeMain

DISK_READ_ERROR:
    MOV AL, 'A'
    MOV AH, 0x0E
    INT 0x10		; This is mostly just for debugging purposes. Eventually I'll make it print out a proper msg

    HLT

gdt_start:

gdt_null:
    DQ 0x0

gdt_code0:
    DW 0xFFFF
    DW 0x0
    DB 0x0
    DB 0x9A
    DB 0b11001111
    DB 0x0

gdt_data0:
    DW 0xFFFF
    DW 0x0
    DB 0x0
    DB 0x92
    DB 0b11001111
    DB 0x0
    
; EVENTUALLY, I will need to add two more descriptors for ring lvl 3 (user mode). The current descriptors are only for
; ring lvl 0 (kernel mode), as I do not have a need for that just yet. Eventually, I will, and eventually, I will addit 
; but for now. Nuh uh!

gdt_end: 


gdtrd:
    DW gdt_end -  gdt_start - 1
    DD gdt_start

BITS 32		       ; protected mode is 32 bits, so we must specify we are now in 32 bits.	
PModeMain:
    MOV AX, 0x10       ; We called PModeMain with 0x08 to set CS to the correct offset. We must now do the same for DS
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    MOV SS, AX    

    MOV AL, 'A'
    MOV AH, 0x4F
    MOV [0xB8000], AX

    JMP 0x08: KERNEL_START_ADDR


    ; once I start trying to jump to kernel, I need to (I think) remember to set EAX to 0x2BADB002 

times 510-($ - $$) db 0     ; this is stuff we use to signal to the BIOS that we want to talk with it
dw 0xAA55
