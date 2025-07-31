BITS 16
ORG 0x7C00

SUPERBLOCK equ 0x8C00   ; superblock starts 4096 bytes after bootloader, which is 0x1000 extra, so 0x8C00
FIRST_INODE_BLOCK equ SUPERBLOCK + 40   ; first_inode_block is 40 bytes after
PREKERNEL_INODE equ 0xB000 + 128        ; we place first_inode_block into memory at 0xB000
KERNEL_INODE equ PREKERNEL_INODE + 128

main:

    MOV AX, 0x0
    MOV SS, AX
    MOV SP, 0x7C00

    MOV BYTE [drive_num], DL    ; DL contains initial boot # on boot up, likely 0x80
    ; I am going to be using Queso Fuego's approach, which is to load in 15 sectors onto the disk. Remember,
    ; 8 sectors = 1 block, first block = bootsector + 2nd stage, bootsector already loaded, so we need 7 sectors
    ; for second stage, and superblock is its own block, so 8 sectors, 7 + 8 = 15 sectors to load.

    ; I will be using LBA addressing this time. (https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h))
    ; aaanndd ill continue this tomorrow. If for some reason it doesn't work out, then boohoo, I'm just gonna
    ; follow queso fuego's bootloader

    ; set up dapack fields for loading 2nd stage & superblock
    MOV WORD [blkcnt], 15
    MOV WORD [db_add], 0x7E00
    MOV WORD [db_add+2], 0x0000
    MOV DWORD [d_lba], 1        ; read from LBA 1 (or sector 1, sector 0 = bootloader)

    MOV SI, DAPACK
    MOV AX, 0x0000
    MOV DS, AX          ; INT 0x13 uses DS:SI to figure stuff out
    MOV DL, [drive_num]
    MOV AH, 0x42        ; DL already contains the drive number, so we're good to go
    INT 0x13
    JC SHORT .error

    ; this wipes the screen clean
    MOV AH, 0x0
    MOV AL, 0x3
    INT 0x10
    
    CALL enable_a20_fast
    CALL inode_table
    CALL load_prekernel
    CALL load_kernel

    MOV DL, [drive_num]
    MOV [0x1500], DL
    JMP 0x0000:0x7E00

.error:
    CLI
.hang:
    HLT
    JMP .hang

enable_a20_fast:
    IN AL, 0x92
    OR AL, 0x02
    AND AL, 0xFE
    OUT 0x92, AL
    RET

inode_table:
    PUSHA
    MOV AX, [FIRST_INODE_BLOCK]
    MOV BX, 8
    MUL BX              ; for this, the result is stored in AX:DX, and since DWORD is 32-bits, we can use AX and DX to store a DWORD into d_lba
    MOV [d_lba], AX     ; 8 sectors per block, first_inode_block contains block #, so block # * 8 = LBA
    MOV [d_lba+2], DX
    MOV WORD [db_add], 0xB000
    MOV WORD [db_add+2], 0x0000
    MOV WORD [blkcnt], 8

    MOV SI, DAPACK
    MOV AX, 0x0000
    MOV DS, AX
    MOV DL, [drive_num]
    MOV AH, 0x42
    INT 0x13
    JC SHORT .error
    POPA
    RET

.error:
    CLI
.hang:
    HLT
    JMP .hang

load_prekernel:
    PUSHA
    MOV AX, [PREKERNEL_INODE+33]
    MOV BX, 8
    MUL BX
    MOV [d_lba], AX
    MOV [d_lba+2], DX

    ; I don't have a size_sectors, and I'm too lazy to add it, so we just use assembly to get it
    MOV AX, [PREKERNEL_INODE+5]         ; the sizes will likely fit into 16-bit registers, none are >65536
    MOV CX, 512
    DIV CX
    CMP DX, 0
    JE .continue
    INC AX      ; ax = quotient, dx = remainder (/ and % respectively)

.continue:
    MOV WORD [blkcnt], AX
    MOV WORD [db_add], 0x0000
    MOV WORD [db_add+2], 0x5000 ; load it into 0x50000

    MOV SI, DAPACK
    MOV AX, 0x0000
    MOV DS, AX
    MOV DL, [drive_num]
    MOV AH, 0x42
    INT 0x13
    JC SHORT .error
    POPA
    RET

.error:
    CLI
.hang:
    HLT
    JMP .hang

load_kernel:
    PUSHA
    MOV AX, [KERNEL_INODE+33]
    MOV BX, 8
    MUL BX
    MOV [d_lba], AX
    MOV [d_lba+2], DX

    MOV AX, [KERNEL_INODE+5]
    MOV CX, 512
    DIV CX
    CMP DX, 0
    JE .continue
    INC AX

.continue:
    MOV WORD [blkcnt], AX
    MOV WORD [db_add], 0x0000
    MOV WORD [db_add+2], 0x1000     ; load it temporarily onto 0x10000

    MOV SI, DAPACK
    MOV AX, 0x0000
    MOV DS, AX
    MOV DL, [drive_num]
    MOV AH, 0x42
    INT 0x13
    JC SHORT .error
    POPA
    RET

.error:
    CLI
.hang:
    HLT
    JMP .hang

DAPACK:
    db 0x10
    db 0
blkcnt: dw 0    ; # of blocks read/written (THESE ARE SECTORS)
db_add: dw 0    ; offset of destination buffer (ex: 0:0x7e00, 0x7e00 is the offset)
    dw 0        ; segment of destination buffer (ex: 0:0x7e00, 0 is the segment)
d_lba:  dd 0    ; the lba to read in this slot
    dd 0

drive_num: db 0

times 510-($ - $$) db 0     ; pad 510 bytes with 0's
dw 0xAA55                   ; the boot signature. Required for the BIOS to recognize what we have is the bootloader
