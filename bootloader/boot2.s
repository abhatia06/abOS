BITS 32
ORG 0x8000

; move lower kernel from 0x10000 to 0x50000
MOV ESI, 0x10000
MOV EDI, 0x50000  ; originally, I was loading it to 0x100000 (1MB), but that is where the higher half kernel will be (in physical memory)
MOV ECX, 4096      ; WHY was I just copying 512 bytes?????
CLD
REP MOVSB

; move higher kernel from 0x20000 to 0x100000
MOV ESI, 0x20000
MOV EDI, 0x100000
MOV ECX, 10752
CLD
REP MOVSB

JMP 0x50000
HLT
