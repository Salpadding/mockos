section .text

%macro gen 1

global ignore_int_%1
ignore_int_%1:
    push %1
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


