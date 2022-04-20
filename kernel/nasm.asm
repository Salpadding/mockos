section .text
extern log_buffer
;log_buffer equ 0x800000
buffer_size equ 2048
global log_buf
global sync_log_buf
global add_loop
global sys_int

sys_int:
    iretd

; int atomic_add(edx)
atomic_add:

add_loop:
    mov eax, [log_buffer]

    mov ecx, eax
    add ecx, edx
    
    cmp ecx, buffer_size 
    jg  .try

    ; cas
    cmpxchg [log_buffer], ecx
    jnz add_loop
    mov eax, ecx
    ret

.try:
    pushf
    test dword [esp], 0x200
    add esp, 4
    ; interrupt is always on in user mode
    jnz add_loop
    push edx
    call sync_log_buf 
    pop edx
    jmp add_loop


; void log_buf(char*, int)
; void log_buf(char*, int)
;              ebp+8, ebp+12
; for printm
log_buf:
    push ebp
    mov ebp, esp

    push esi
    push edi
    push ds
    push es

    mov  ax, 0x1b
    mov  ds, ax 

    mov  edx, [ebp + 12]    
    call atomic_add

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
    add edi, log_buffer + 16
    sub edi, ecx
    cld
    rep
    movsb
.ret:
    ; restore registers
    pop es
    pop ds
    pop edi
    pop esi
    pop  ebp
    ret


sync_log_buf:
    mov  ecx, [log_buffer]
    test ecx, ecx
    jz .rt

    push edi
    mov  edi, log_buffer + 16

    push ecx
    push edi

; \0 means string copy not completed
    mov ax, 0
    cld
    repne scasb
    jz .check_failed

    pop edi
    pop ecx

    mov  dx, 0x3f8 + 5
.ready:
    in  al, dx
    test al, 0x20
    jz .ready

.write:
    mov  dx, 0x3f8
    mov  ax, [edi] 
    mov byte [edi], 0
    out  dx, ax
    inc  edi
    mov  dx, 0x3f8 + 5
    loop .ready

    mov dword [log_buffer], 0
    pop edi
.rt:
    ret
.check_failed:
    add esp, 8
    pop edi
    ret
    


