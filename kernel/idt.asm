section .text

global hd_interrupt
extern hd_timeout ; 一个全局变量
extern do_hd ; 一个函数指针
extern unexpected_hd_int; 

%macro gen 1

global ignore_int_%1
ignore_int_%1:
    push %1
.lp:
    mov eax, %1
    jmp .lp
    push eax
    push ecx
    push edx
    push ds
    push es
    push fs
    
    mov eax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    
    pop fs
    pop es
    pop ds
    pop edx
    pop ecx
    pop eax
    add esp, 4
    iret							

%endmacro

%macro ident 1
    dd ignore_int_%1
%endmacro

%assign i 0
%rep 256
    gen i
%assign i i+1
%endrep


section .data
global idt_ptrs
idt_ptrs:

%assign i 0
%rep 256
    ident i
%assign i i+1
%endrep


global hd_interrupt

hd_interrupt:
    push eax
    push ecx
    push edx
    push ds
    push es
    push fs
    mov eax, 0x10
    mov ds, ax
    mov es, ax
    mov eax, 0x17
    mov fs, ax
    mov al, 0x20
    out 0xa0, al
    jmp .l1
.l1:
    jmp .l2
.l2:
    ; 这里用了一个乐观锁
    xor edx, edx 
    mov [hd_timeout], edx
    xchg edx, [do_hd]
    test edx, edx
    jne .l3 ; 当 edx 
    mov  edx, unexpected_hd_int
.l3:
    out 0x20, al
    call edx
    pop fs
    pop es
    pop ds
    pop edx
    pop ecx
    pop eax
    iret


