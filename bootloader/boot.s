BITS 16
ORG 0x7C00            ; this address is where the BIOS starts reading from during boot.

KERNEL_LOAD_SEG equ 0x1000
KERNEL_START_ADDR equ 0x100000

start:
    JMP main

loadKernelToMem:
    MOV DH, 0x0
    MOV DL, 0x80
    MOV CL, 0x03
    MOV CH, 0x0
    MOV AX, KERNEL_LOAD_SEG
    MOV ES, AX
    MOV BX, 0x0000
    MOV AH, 0x02
    MOV AL, 1
    MOV DI, 3

.retry
    STC
    INT 0x13             ; The final piece of the puzzle. Interrupt 0x13 is for the BIOS Disk Service
    JC DISK_READ_ERROR  ; BIOS sets the carry flag if the call failed. JC jumps if carry flag is set.
    RET

puts:
    PUSH SI
    PUSH AX

.loop:
    LODSB
    CMP AL, 0
    JE .done
    MOV AH, 0x0E
    INT 0x10
    JMP .loop

.done:
    POP AX
    POP SI
    RET

main:
    CLI
    MOV AX, 0x0         ; We put 0x0 into AX because we cant directly put values into the segment registers
    MOV SS, AX          ; SS:SP is a segment for the stack segment. We are trying to basically initialize our stack
    MOV SP, 0x7C00      ; We initialize our stack at 0x7C00
    STI

    ; The code below follows the CHS reading format
    ; https://stackoverflow.com/questions/67613202/bios-interrupt-13-read-sector-not-working
    MOV DH, 0x0         ; We want to read from Head 0
    MOV DL, 0x80        ; We want to read from  the first hard drive (0x80)
    MOV CL, 0x02        ; We want to start from the 2nd sector (first sector is bootloader)
    MOV CH, 0x0         ; We want to read from cylinder 0
    MOV AX, 0x800
    MOV ES, AX
    MOV BX, 0x0000
    MOV AH, 0x02
    MOV AL, 1          ; The number of sectors we want to read (20).
    MOV DI, 3

retry:
   STC
   INT 0x13             ; The final piece of the puzzle. Interrupt 0x13 is for the BIOS Disk Service
   JC DISK_READ_ERROR  ; BIOS sets the carry flag if the call failed. JC jumps if carry flag is set.
   MOV AH, 0x0
   MOV AL, 0x3
   INT 0x10

   CALL loadKernelToMem
   JMP enterProtectedMode

.fail:
   PUSH DS
   PUSH SI
   JMP DISK_READ_ERROR

.done:
   JMP enterProtectedMode

enterProtectedMode:
    CLI
    LGDT [gdtrd]
    MOV EAX, CR0       ; We need to set the PE bit to 1 on the CR0 register to switch to protected mode, and we do it
    OR EAX, 1          ; through these three registers.
    MOV CR0, EAX
    JMP 0x08:PModeMain

DISK_READ_ERROR:
    MOV SI, msg_read_failed
    CALL puts
    JMP wait_key_and_reboot

wait_key_and_reboot:
    MOV AH, 0
    INT 0x16
    JMP 0xFFFF0000

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

msg_read_failed:        db 'Read From Disk Failed', 0

BITS 32                ; protected mode is 32 bits, so we must specify we are now in 32 bits.
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

    JMP 0x08: 0x8000 ;

times 510-($ - $$) db 0     ; this is stuff we use to signal to the BIOS that we want to talk with it
dw 0xAA55
