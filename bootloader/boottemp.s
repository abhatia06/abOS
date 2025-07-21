; I have decided to abandon trying to switch over to using ATA PIO to load up the bootloader. It isn't strictly necessary, but it is nice to do
; to understand ATA PIO. For some reason, this "new" bootloader doesn't work, as it only seems to copy 512 bytes (256 words, or 1 sector), instead
; of doing the number of sectors you would expect..... I don't fully know WHY this is happening? I might come back and try to fix this later in the future,
; who knows. But for now, we continue to use CHS addressing with INT 0x13 (very outdated, lol)

BITS 16
ORG 0x7C00

KERNEL_LOAD_SEG equ 0x1000
KERNEL_START_ADDR equ 0x100000

; TODO: REMOVE MAGIC NUMBERS!

start:
    JMP main


main:
    CLI
    MOV AX, 0x0
    MOV SS, AX
    MOV SP, 0x7C00
    STI

    CALL load2ndstage
    CALL loadprekernel
    CALL loadkernel
    CALL enable_a20_fast
    JMP 0x0000:0x7E00

load2ndstage:
    PUSH CX
    PUSH BX
    PUSH AX

    MOV CH, 1           ; read 1 sector
    MOV BL, 0x03        ; starting sector 3
    XOR AX, AX
    MOV ES, AX
    XOR AX, AX
    MOV AX, 0x7E00
    MOV DI, AX          ; ES:DI = 0x7E00
    CALL ata_chs_read

    POP AX
    POP BX
    POP CX
    RET

loadprekernel:
    PUSH CX
    PUSH BX
    PUSH AX

    MOV CH, 10          ; read 10 sectors
    MOV BL, 0x1F        ; starting sector 31
    XOR AX, AX
    MOV AX, 0x2000
    MOV ES, AX
    XOR AX, AX
    MOV DI, AX
    CALL ata_chs_read

    POP AX
    POP BX
    POP CX
    RET

loadkernel:
    PUSH CX
    PUSH BX
    PUSH AX

    MOV CH, 27
    MOV BL, 0x04
    XOR AX, AX
    MOV AX, KERNEL_LOAD_SEG
    MOV ES, AX
    XOR AX, AX
    MOV DI, AX
    CALL ata_chs_read

    POP AX
    POP BX
    POP CX
    RET

enable_a20_fast:
    IN AL, 0x92
    OR AL, 0x02
    AND AL, 0xFE
    OUT 0x92, AL
    RET

;
; ATA read sectors (no int 13h) w/ CHS mode. In the future we'll use LBA, but I don't really get LBA, cuz I haven't
; studied it, so we stick to CHS for now.
; CH = number of registers to read,
; BL = sector index
;
; this version of ATA read sectors is heavily limited by the fact we cannot use 32-bit registers, and thus, our
; maximum memory we can write to is <1MB, and the maximum number of sectors we can read is 256 sectors.
ata_chs_read:
    PUSHF
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    PUSH DI

    MOV DX, 0x1F6       ; head & drive # port
    MOV AL, 0x80        ; drive # - (0x80 is hard disk 1)
    MOV AL, 0x00        ; head # (low bytes) (head 0)
    OR AL, 0xA0         ; default high bytes for master drive (drive 1) (for slave it's 0xB0)
    OUT DX, AL

    MOV DX, 0x1F2       ; sector count port
    MOV AL, CH          ; # of sectors to read
    OUT DX, AL

    MOV DX, 0x1F3       ; sector index port
    MOV AL, BL          ; sector index to begin reading from
    OUT DX, AL

    MOV DX, 0x1F4       ; cylinder low port
    XOR AL, AL
    OUT DX, AL

    MOV DX, 0x1F5       ; cylinder high port
    XOR AL, AL
    OUT DX, AL          ; we're going to be reading from cylinder 0 only b/c we only wanna load in kernel rn

    MOV DX, 0x1F7       ; command port
    MOV AL, 0x20        ; read w/ retry command
    OUT DX, AL

.loop:
    IN AL, DX           ; read port 0x1F7 (status register)
    TEST AL, 8          ; see if sector buffer is ready
    JE .loop            ; poll until its ready

    MOV AX, 256         ; 256 words = 1 sector (512 bytes)
    XOR BX, BX
    MOV BL, CH
    MUL BL              ; DX:AX = AX * CH, or 256 * number of sectors we wanna read
    MOV CX, AX          ; cx is counter for INSW
    MOV DX, 0x1F0       ; data port
    REP INSW            ; read bytes from dx port # into memory address DI, CX # of times

    POP DI
    POP DX
    POP CX
    POP BX
    POP AX
    POPF
    RET

times 510-($ - $$) db 0         ; pad 510 bytes with 0's
dw 0xAA55                       ; boot signature
