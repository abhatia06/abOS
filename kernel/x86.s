BITS 32

VIDEO_MEMORY equ 0xB8000
WHITE_ON_BLACK equ 0x0F ; video memory works as: 0xB8000 = first character, 0xB8001 = color for first character, and then loops

SECTION _text:


GLOBAL _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:

     PUSH EBP                    ; Nanobyte uses BP, and a lot of other people use BP too, so  I decided to use it as well. Here we save the old frame data
     MOV EBP, ESP                ; And we load in the new frame data 

; ESP + 8 is the character we want to display. ESP is the stack pointer, it points to the return address since this function is
; being called. But, we also pushed EBP into the stack, so ESP actually now points to EBP. Also, since we're using CDECL, and each argument
; was passed from right to left, that means the argument that is furthest away from ESP is the position variable, followed by the page variable, followed by the character
; finally followed by the return address. Since each element in the stack takes up 4 bytes, we have to add 4 at a time to skip certain things in the stack. Hence, we do
; ESP + 8 for the character.

     MOVZX EAX, BYTE [EBP + 8] 
     MOV EBX, VIDEO_MEMORY
     ADD EBX, [EBP + 16]     ; offset 
     MOV AH, WHITE_ON_BLACK
     MOV EBX, AX
     
     POP EBP          ; Return EBP back to its original address
     RET
