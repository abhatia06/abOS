ORG 0x7E00
BITS 16

KERNEL_LOAD_SEG equ 0x1000
KERNEL_START_ADDR equ 0x100000
MEMORY_MAP_ADDR equ 0x9000
MEMORY_MAP_ENTRY_SIZE equ 24

main:
   CALL do_e820
   CALL enterProtectedMode 

mmap_ent equ 0x9000
do_e820:
    XOR AX, AX
    MOV ES, AX
    MOV DI, 0x9004
    XOR EBX, EBX
    XOR BP, BP
    MOV EDX, 0x0534D4150
    MOV EAX, 0xE820
    MOV [ES:DI + 20], DWORD 1
    MOV ECX, 24
    INT 0x15
    JC .error

    MOV EDX, 0x0534D4150
    CMP EAX, EDX
    JNE .error

    TEST EBX, EBX
    JE .error

    JMP .jmpin

.e820lp:
    MOV EAX, 0xE820
    MOV [ES:DI + 20], DWORD 1
    MOV ECX, 24
    INT 0x15
    JC .e820f
    MOV EDX, 0x0534D4150

.jmpin:
    JCXZ .skipentry
    CMP CL, 20
    JBE .notext
    TEST BYTE [ES:DI + 20], 1
    JE .skipentry

.notext:
    MOV ECX, [ES:DI + 8]
    OR ECX, [ES:DI + 12]
    JZ .skipentry
    INC BP
    ADD DI, 24

.skipentry:
    TEST EBX, EBX
    JNE .e820lp

.e820f:
    MOV [ES:mmap_ent], bp
    CLC
    RET

.error:
    STC
    RET


enterProtectedMode:
    ;MOV AH, 0x0
    ;INT 0x16
    CLI
    LGDT [gdtrd]
    MOV EAX, CR0       ; We need to set the PE bit to 1 on the CR0 register to switch to protected mode, and we do it
    OR EAX, 1          ; through these three registers.
    MOV CR0, EAX
    JMP 0x08:PModeMain


TSS:
    dd 0h
    dd 100000h
    dd 10h
    dd 0        ;esp1
    dd 0        ;ss1
    dd 0        ;esp2
    dd 0        ;ss2
    dd 0        ;cr3
    dd 0        ;eip
    dd 0        ;eflags
    dd 0        ;eax
    dd 0        ;ecx
    dd 0        ;edx
    dd 0        ;ebx
    dd 0        ;esp
    dd 0        ;ebp
    dd 0        ;esi
    dd 0        ;edi
    dd 0        ;es
    dd 0        ;cs
    dd 0        ;ss
    dd 0        ;ds
    dd 0        ;fs
    dd 0        ;gs
    dd 0        ;ldt
    dw 0        ;trap
    DW 0        ;iomap
.end:

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

gdt_code3:
    DW 0xFFFF
    DW 0x0
    DB 0x0
    DB 0xFA
    DB 0b11001111
    DB 0x0

gdt_data3:
    DW 0xFFFF
    DW 0x0
    DB 0x0
    DB 0xF2
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

    JMP 0x08: 0x8000 
