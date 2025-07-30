ORG 0x7E00
BITS 16

KERNEL_LOAD_SEG equ 0x1000
KERNEL_START_ADDR equ 0x100000
MEMORY_MAP_ADDR equ 0xA000
MEMORY_MAP_ENTRY_SIZE equ 24

main:
   CALL do_e820
   CALL enterProtectedMode

mmap_ent equ 0xA000
do_e820:
    XOR AX, AX
    MOV ES, AX
    MOV DI, 0xA004
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
    JMP 0x08:PModeMain ; far jump


TSS:
    dd 0x0      ;(reserved) link
    dd 0xC0003808 ;esp0, top of the kernel stack (stack_end)
    dd 0x10     ;(reserved) ss0
    dd 0        ;esp1
    dd 0        ;(reserved) ss1
    dd 0        ;esp2
    dd 0        ;(reserved) ss2
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
    dd 0        ;(reserved) es
    dd 0        ;(reserved) cs
    dd 0        ;(reserved) ss
    dd 0        ;(reserved) ds
    dd 0        ;(reserved) fs
    dd 0        ;(reserved) gs
    dd 0        ;(reserved) ldtr
    dw 0        ;iobp (reserved)
    DW 0        ;ssp
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

; offset 0x28
; this TSS is also ripped off from Queso Fuego. I don't know why he does it like this, though
tss:
    DW TSS.end - TSS - 1        ; first 16 bits - sizeof(TSS)-1
    DW TSS                      ; next 16 bits is the base, which is &TSS
    DB 0x0
    DB 0x89                     ; access byte, 8 bits, 0x89 according to OSDev
    DB 0x0                      ; flags = 0
    DB 0x0
; We want to divide the limit and base to be like low bytes, medium bytes, high bytes, but that isn't possible
; because the NASM assembler doesn't allow us to use bit-wise shifts (>>) or bit-wise anding or oring (& |). So,
; we have to put the tss stuff to 0 for right now,

gdt_end:


gdtrd:
    DW gdt_end -  gdt_start - 1
    DD gdt_start

msg_read_failed:        db 'Read From Disk Failed', 0
TSS_BASE equ 0xB000

BITS 32                ; protected mode is 32 bits, so we must specify we are now in 32 bits.
PModeMain:
    MOV AX, 0x10       ; We called PModeMain with 0x08 to set CS to the correct offset. We must now do the same for DS
    MOV DS, AX
    MOV ES, AX
    MOV FS, AX
    MOV GS, AX
    MOV SS, AX

    MOV EAX, gdt_start
    MOV [0x1810], EAX   ; check global_addresses.h

    MOV ESI, 0x10000
    MOV EDI, 0x100000
    MOV ECX, 25000
    CLD
    REP MOVSB

    MOV ESI, 0x20000
    MOV EDI, 0x50000
    MOV ECX, 10000
    CLD
    REP MOVSB

    JMP 0x08: 0x50000
    HLT
