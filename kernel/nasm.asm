section .text
extern log_buffer
;log_buffer equ 0x800000
buffer_size equ 2048
global log_buf
global sync_log_buf
global sync_log_buf_2
global add_loop
global sys_int
global atomic_add_try

sys_int:
    iretd

; int atomic_add(edx)
atomic_add:

add_loop:
    mov eax, [log_buffer]

    mov ecx, eax
    add ecx, edx
    
    cmp ecx, buffer_size 
    jg  atomic_add_try

    ; cas
    cmpxchg [log_buffer], ecx
    jnz add_loop
    mov eax, ecx
    ret

atomic_add_try:
    pushf
    test dword [esp], 0x200
    add esp, 4
    ; interrupt is always on in user mode
    jnz add_loop
    push edx
    call sync_log_buf_2
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
    mov  al, [edi] 
    mov byte [edi], 0
    out  dx, al
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
    


; [log_buffer] = limit
; [log_buffer+8] = offset
sync_log_buf_2:
    push ecx
    mov  ecx, [log_buffer]
    sub  ecx, [log_buffer + 8]
    test ecx, ecx
    jz .rt

    push ebx
    mov  ebx, ecx
    push esi
    mov  esi, log_buffer + 16
    ; add offset
    add  esi, [log_buffer + 8]

    mov  dx, 0x3f8 + 5

.ready:
    in  al, dx
    test al, 0x20
    jz .write_end

.write:
    mov  dx, 0x3f8
    mov  al, [esi] 
    cmp  ax, 0
    jz .write_end
    out  dx, al
    inc  esi
    mov  dx, 0x3f8 + 5
    loop .ready
.write_end:
    test ecx, ecx
    jnz  .not_completed
    mov dword [log_buffer], 0
    mov dword [log_buffer + 8], 0
    jmp .clean
.not_completed:
    sub ebx, ecx
    add [log_buffer + 8], ebx
.clean:
    pop esi
    pop ebx
.rt:
    pop ecx
    ret
    


