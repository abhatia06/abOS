BITS 16
ORG 0x7C00

KERNEL_LOAD_SEG equ 0x1000

; for future reference:
; PREKERNEL - START AT SECTOR 0x1F, 10 SECTORS LARGE
; KERNEL    - START AT SECTOR 0x04, 27 SECTORS LARGE
; 2ND STAGE - START AT SECTOR 0x02, 1 SECTOR LARGE

start: 
    JMP main


main:
    CLI 
    MOV AX, 0x0
    MOV SS, AX
    MOV SP, 0x7C00
    STI


;
; ATA read sectors (no int 13h) w/ CHS mode. In the future we'll use LBA, but I don't really get LBA, cuz I haven't
; studied it, so we stick to CHS for now.  
; CH = number of registers to read, 
; BL = sector index
; SET ES:DI BEFOREHAND TO PICK LOCATION IN MEMORY TO PUT THE ATA READ
; 
ata_chs_read:
    PUSHF
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    PUSH DI

    MOV DX, 0x1F6	; head & drive # port
    MOV AL, 0x80	; drive # - (0x80 is hard disk 1)
    MOV AL, 0x0F	; head # (low bytes)
    OR AL, 0x0A0	; default high bytes for master drive (drive 1) (for slave it's 0x0B0)
    OUT DX, AL

    MOV DX, 0x1F2	; sector count port
    MOV AL, CH		; # of sectors to read
    OUT DX, AL

    MOV DX, 0x1F3	; sector index port
    MOV AL, BL 		; sector index to begin reading from
    MOV DX, AL 
	
    MOV DX, 0x1F4       ; cylinder low port
    XOR AL, AL
    OUT DX, AL

    MOV DX, 0x1F5       ; cylinder high port
    XOR AL, AL
    OUT DX, AL		; we're going to be reading from cylinder 0 only b/c we only wanna load in kernel rn 

    MOV DX, 0x1F7	; command port 
    MOV AL, 0x20	; read w/ retry command
    OUT DX, AL 

.loop 
    IN AL, DX		; read port 0x1F7 (status register)
    TEST AL, 8		; see if sector buffer is ready
    JE .loop		; poll until its ready 

    MOV AX, 256		; 256 words = 1 sector
    XOR BX, BX
    MOV BL, CH
    MUL BL		; DX:AX = AX * CH, or 256 * number of sectors we want to read 
    MOV CX, AX
    MOV DX, 0x1F0	; data port 
    REP INSW		; reads from ES:DI 
    
    POP DI
    POP DX
    POP CX
    POP BX
    POP AX
    POPF
    RET  

times 510-($ - $$) db 0		; pad remaining bytes with 0 to make bootloader full 512 bytes
dw 0xAA55			; boot signature
