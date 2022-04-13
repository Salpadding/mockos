section .text
global atomic_add
global log_buf

; int atomic_add(int*, int)
atomic_add:
    push ebx
    push ecx
    mov ebx, [esp+12]
.loop:
    mov eax, [ebx]
    mov ecx, eax
    add ecx, [esp + 16] 
    
    ; cas
    cmpxchg [ebx], ecx
    jnz .loop
    mov eax, ecx
    pop ecx
    pop ebx
    ret
    mov eax, 0xffffffff
; void log_buf(char*, int)
; void log_buf(char*, int)
;              ebp+8, ebp+12
log_buf:
    push ebp
    mov ebp, esp

    push ecx
    push esi
    push edi
    push ds
    push es

    mov  eax, 0x1b
    mov  ds, ax 

    ; call atomic add
    push dword [ebp + 12]
    push 0x800000
    call atomic_add
    add  esp, 8
    push eax

    ; copy memory

    ; source segment
    mov ax, ss
    mov ds, ax

    ; dest  segment
    mov ax, 0x1b
    mov es, ax

    mov ecx, [ebp + 12]
    mov esi, [ebp + 8]
    pop edi
    add edi, 0x800000 + 16
    sub edi, ecx
    rep
    movsb

    ; restore registers
    pop es
    pop ds
    pop edi
    pop esi
    pop ecx
    pop  ebp
    ret
