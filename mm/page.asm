global page_fault
extern do_wp_page

section .text
page_fault:
    xchg [esp], eax ; 取出错误码, 同时把 eax 入栈
    push ecx
    push edx
    push ds
    push es
    push fs
    mov  edx, 0x10
    mov  ds, dx
    mov  es, dx
    mov  fs, dx
    mov  edx, cr2
    push edx
    push eax
    call do_wp_page 
    add  esp, 8
    pop  fs
    pop  es
    pop  ds
    pop  edx
    pop  ecx
    pop  eax
    iret
