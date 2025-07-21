; this bootloader is insanely unorganized and messy. For an actually readable (though, not working, for some reason??) bootloader, look at 
; exampleATAbootload.s, as that bootloader uses ATA PIO instead of INT 0x13 to load from disk to memory (again, though, it doesn't work fully)
BITS 16
ORG 0x7C00            ; this address is where the BIOS starts reading from during boot.

KERNEL_LOAD_SEG equ 0x1000
KERNEL_START_ADDR equ 0x100000

start:
    JMP main

loadPrekernel:
    MOV DH, 0x0
    MOV DL, 0x80
    MOV CL, 0x1F
    MOV CH, 0x0
    MOV AX, 0x2000
    MOV ES, AX
    MOV BX, 0x0000
    MOV AH, 0x02
    MOV AL, 10
    MOV DI, 3

.retry:
    STC
    INT 0x13
    JC DISK_READ_ERROR
    RET

; For the CPU to process data from disk, (which is what we're doing), the data must first be transfered over to the
; main memory, (or RAM) by CPU-generated I/O calls, (which is again, what we're doing). But we need the I/O call to
; KNOW what part of the disk we want to access and store into memory, (if we put the entire disk, memory would likely
; blow up, since RAM is intended to be faster in terms of access, but smaller in size compared to the disk). So, we use
; CHS addressing for that.
loadKernelToMem:
    MOV DH, 0x0
    MOV DL, 0x80
    MOV CL, 0x04
    MOV CH, 0x0
    MOV AX, KERNEL_LOAD_SEG
    MOV ES, AX
    MOV BX, 0x0000
    MOV AH, 0x02
    MOV AL, 27
    MOV DI, 3

.retry:
    STC
    INT 0x13             ; The final piece of the puzzle. Interrupt 0x13 is for the BIOS Disk Service
    JC DISK_READ_ERROR  ; BIOS sets the carry flag if the call failed. JC jumps if carry flag is set.i
    RET

load2ndstage:
    MOV DH, 0x0
    MOV DL, 0x80
    MOV CL, 0x02
    MOV CH, 0x0
    MOV AX, 0x0000
    MOV ES, AX
    MOV BX, 0x7E00      ; Second stage bootloader is gonna be at 0x7E00
    MOV AH, 0x02
    MOV AL, 1
    MOV DI, 3

.retry:
    STC
    INT 0x13
    JC DISK_READ_ERROR
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
    MOV CL, 0x03        ; We want to start from the 3rd sector (first sector is bootloader)
    MOV CH, 0x0         ; We want to read from cylinder 0
    MOV AX, 0x800
    MOV ES, AX
    MOV BX, 0x0000
    MOV AH, 0x02
    MOV AL, 1          ; The number of sectors we want to read (1).
    MOV DI, 3

.mainretry:
    STC
    INT 0x13            ; The final piece of the puzzle. Interrupt 0x13 is for the BIOS Disk Service
    MOV SI, message_test
    CALL puts
    JC DISK_READ_ERROR  ; BIOS sets the carry flag if the call failed. JC jumps if carry flag is set.
    MOV AH, 0x0
    MOV AL, 0x3
    INT 0x10

    CALL loadKernelToMem
    CALL loadPrekernel
    CALL load2ndstage
    CALL enable_a20_fast
    JMP 0x0000:0x7E00

.fail:
   PUSH DS
   PUSH SI
   JMP DISK_READ_ERROR

enable_a20_fast:
    IN AL, 0x92
    OR AL, 0x02
    AND AL, 0xFE
    OUT 0x92, AL
    RET

DISK_READ_ERROR:
    MOV SI, msg_read_failed
    CALL puts
    JMP wait_key_and_reboot

wait_key_and_reboot:
    MOV AH, 0
    INT 0x16
    JMP 0xFFFF0000


msg_read_failed:        db 'Read From Disk Failed', 0
message_test:           db 'test', 0
times 510-($ - $$) db 0     ; pad 510 bytes with 0's
dw 0xAA55                   ; the boot signature. Required for the BIOS to recognize what we have is the bootloader
