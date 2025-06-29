BITS 32
ORG 0x8000

MOV ESI, 0x10000
MOV EDI, 0x50000  ; originally, I was loading it to 0x100000 (1MB), but that is where the higher half kernel will be (in physical memory)
MOV ECX, 4096      ; WHY was I just copying 512 bytes?????
CLD
REP MOVSB
JMP 0x100000
HLT
