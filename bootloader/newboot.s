BITS 16
ORG 0x7C00

main:
    MOV BYTE [drive_num], DL    ; DL contains initial boot # on boot up, likely 0x80
    ; I am going to be using Queso Fuego's approach, which is to load in 15 sectors onto the disk. Remember,
    ; 8 sectors = 1 block, first block = bootsector + 2nd stage, bootsector already loaded, so we need 7 sectors
    ; for second stage, and superblock is its own block, so 8 sectors, 7 + 8 = 15 sectors to load.
    MOV AL, 15
    MOV BL, AL
    DEC BL
    MOV DI, 0x7E00      ; read sectors to 0x7E00

    ; I will be using LBA addressing this time. (https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h))
    ; aaanndd ill continue this tomorrow. If for some reason it doesn't work out, then boohoo, I'm just gonna
    ; follow queso fuego's bootloader


DAPACK:
    db 0x10
    db 0
blkcnt: dw 0    ; # of blocks read/written
db_add: dw 0    ; memory buffer destination address (ex: dw 0x7E00 would be 0:7e00)
    dw 0
d_lba:  dd 1    ; the lba is in read
    dd 0


times 510-($ - $$) db 0     ; pad 510 bytes with 0's
dw 0xAA55                   ; the boot signature. Required for the BIOS to recognize what we have is the bootloader
