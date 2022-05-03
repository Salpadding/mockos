# Linux 0.12 kernel notes

## 为什么栈指针从高地址向低地址增长?

因为 eip 是从低地址向高地址增长的, 这样可以充分利用内存空间

## 为什么 c 语言入参是从右往左入栈的?

其实这个说法不对, 不是 c 语言规定了要从右往左入栈, 而是人们阅读的习惯是左边的是低地址右边是高地址，而栈指针又是从高向低增长, 
所以给人产生了 c 语言从右往左入参的感觉

## setup.S.S:173 

The lidt and lgdt load the interrupt descriptor table and global descriptor table.

the operand lidt and lgdt is a pointer like below:

```c
struct entry {
   char[8] bits;
};
struct table_info {
    char[2] len;// length of table in bytes - 1, however, the first entry is ignored
    struct entry* entries; // double word, 32bit pointer 
};
```

这里的 lgdt 存在段错误隐患, 因为 gdt 的长度是 0x800 个字节, 而实际上定义了 2 个 gd, 定义的部分只有 8 * 3 个字节, 存在溢出的部分
lidt 没有产生段错误, 因为 idt 长度是 0


## head.s:337

head.s 中重新定义了 idt 和 gdt, idt 是一个 256 表项的空表, gdt 预分配了 256 个表项, 前两个表项分别是内核代码段和内核数据段, 都限制在了 16mb 
```asm
idt_descr:
    .word 256 * 8 - 1				# idt contains 256 entries
    .long idt

.align 4    # 这个对齐貌似多余

.word 0
gdt_descr:
    .word 256 * 8 - 1				# so does gdt (not that that's any
    .long gdt						# magic number, but it works for me :^)

.align 8
```

这里定义的 idt 和 gdt 没有段错误, 因为下面的 idt 和 gdt 都填充足够 256 * 8 个字节

```asm
# 中断描述符表（空表）
idt:	.fill 256, 8, 0					# idt is uninitialized

# 全局描述符表
# 前4项分别是空项(不用)、代码段描述符、数据段描述符、系统调用段描述符（没有使用）
# 同时还预留了252项的空间，用于放置所创建任务的局部描述符(LDT)和对应的任务状态段TSS的描述符
# (0-nul, 1-cs, 2-ds, 3-syscall, 4-TSS0, 5-LDT0, 6-TSS1, 7-LDT1, 8-TSS2 etc...)
gdt:
    .quad 0x0000000000000000			/* NULL descriptor */
    .quad 0x00c09a0000000fff			/* 16Mb */		# 0x08，内核代码段，长度16MB
    .quad 0x00c0920000000fff			/* 16Mb */		# 0x10，内核数据段，长度16MB
    .quad 0x0000000000000000			/* TEMPORARY - don't use */
    .fill 252, 8, 0						/* space for LDT's and TSS's etc */
```


## head.s:294

1. enable paging, the page directory is located at 0x08
2. allocate 4 page table [4k, 8k, 12k, 16k]
3. kernel page mapping = [0-16m] -> [0-16m]
4. ret 指令会跳转到 main 函数

## memory.c:667

初始化内存管理

1. memory start = 4M, memory end = 16M
2. `mem_map` 是以页为单位的 byte map, 长度为 `15M / 4K`, 15M 是 [1M, 16M) 之间的内存
3. `mem_init` 的作用是把 [1M, 4M) 之间的内存设置为已使用 
4. memory dump 显示 [0x26c00, 0x26F00) 之间的内存被设置为 0x0D, 正好对应上了 [1M, 4M) [0x26c00, 0x26f00) -> [1M, 4M)

## traps.c:212

初始化中断门

system.h 中定义了 _set_gate 宏, idt 引用了全局符号, 导出自 head.s, 地址为 0x54c0
经过 print 验证 c 语言中 idt 指针也是 0x54c0


## system.h:33

set_trap_gate(0, 0x81c6) 也就是 _set_gate(&idt[0], 15, 0, 0x81c6) 的结果是

idt[0] = 0x00008f00000881c6

这里 0 表示特权级为 0
15 也就是 0xF 表示这是一个 32bit trap gate

通过位运算验证结果:

1. (idt[0] & 0xffff) == 0x81c6, 因为最高16位都是0所以忽略了
2. (idt[0] & (0xffff << 16)) >> 16  segment selector = 8, 也就是 gdt 中指向 code 的段选择子
3. (idt[0] & (0xf) << 40) >> 40  gatetype = 0xf  trap gate 
4. (idt[0] & (1 << 44)) = 0 这位没有用途
5. (idt[0] & (3 << 45)) >> 45 = 0 dpl 等于 0 
6. (idt[0] & 1 << 47) != 0,  present bit

## 进入用户态

1. 进入用户态之前需要做哪些准备?

进入用户态后，将来肯定是要回到内核态的，所以要在 gdt 中定义 tss, 加载到 tr 寄存器，这样将来回到内核态时才能加载上下文 

2. 进入用户态后哪些寄存器发生了改变?

eip, cs, eflags, esp, ss

3. 如何进入用户态?

把 NT flag 清除, 使用 iretd 可以向降低特权级跳转, 从而进入用户态, 因为是向其他特权级跳转所以 iretd 会依次 pop 五个参数分别是

eip, cs, eflags, esp, ss

进入用户态之前, 把内核上下文保存在 tss 中, sched.c:557
调试发现必须设置 ldt 后才可以进入用户态
linux0.12 在 iretd 前使用 lldt 设置了 ldt, 所以实际上用户态的特权级是 ldt 里面的 cs 和 ss 决定的

main.c:214

```asm
mov    eax,esp
push   0x17 // 这一步的选择子使用的是 ldt 而不是 gdt
push   eax
pushfd        ; push eflags
push   0xf    ; cs
push   0x6853 ; eip
iretd ; pop to eip, cs and eflags, esp, ss
; iretd 从栈 pop 了 5 个 dw
```

经调试发现 iretd 后

eip: 0x6852 -> 0x6853
cs: 0x08 -> 0x0f
esp: 0x37c67 -> 0x37c08
ds, es, fs, gs: 0x10 -> 0x00
general register 不变
ss: 0x10 -> 0x17

cs: index 还是 1, 但是 rpl 变成了 3
ss: index 还是 1, 但是 rpl 变成了 3

cpl 从 0 变成了 3

CPL = CS & 3 = 3


iretd 后面是

```asm
mov eax, 0x17
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
```

ds, es, fs, gs:
index: 1 -> 1
rpl:   0 -> 3

linux0.12 没有定义 dpl = 3 的 segment descriptor

## 系统调用

如何进入内核态?

通过 int 指令

进入内核态后用户的上下文如何保存?

用户态的 eip, cs, eflags 会被 push 到栈上, 其他寄存器需要在中断处理函数中手动保存

1. the system call service routine registered by sched.c:578
2. 以调用 fork 为例, 系统调用的第一步是从用户态进入内核态


```asm
mov eax, 2
int 0x80
```

3. 中断前后寄存器状态

before int 0x80 和 after int 0x80 后变化的有如下寄存器(按照入栈顺序排序)

4. ss, esp, eflags, cs, eip
反过来就是 iretd 的出栈顺序, 其中 eip 是 int 0x80 的下一条指令的 eip

内存dump发现进入内核态之前tss的状态如下:
back_link = 0x0
esp0 = 0x25c80
ss0 = 0x10
esp1 = 0x0
ss1 = 0x0
esp2 = 0x0
ss2 = 0x0
cr3 = 0x0
eip = 0x0
eflags = 0x0
eax = 0x0
ecx = 0x0
edx = 0x0
ebx = 0x0
esp = 0x0
ebp = 0x0
esi = 0x0
edi = 0x0
es = 0x17
cs = 0x17
ss = 0x17
ds = 0x17
fs = 0x17
gs = 0x17
ldt = 0x28

5. 进入中断从先从 idt 加载cs eip 作跨特权级跳转, 然后从 tss 加载 esp0 和 ss0, tss 中有用的也只有 esp0 和 ss0

## main.c:216 fork()

系统调用的过程, move_to_user_mode() 进入了用户态, fork() 函数通过 int 0x80 进入了内核态

```asm
system_call:
push   ds
push   es
push   fs
push   eax
push   edx
push   ecx
push   ebx
mov    edx,0x10
mov    ds,edx
mov    es,edx
mov    edx,0x17
mov    fs,edx
cmp    eax,DWORD PTR ds:0x0
jae    0 bad_sys_call
call   DWORD PTR [eax*4+0x0]
push   eax
mov    eax,ds:0x0
cmp    DWORD PTR [eax],0x0
jne    4 reschedule
cmp    DWORD PTR [eax+0x4],0x0
je     4 reschedule

ret_from_sys_call:
mov    eax,ds:0x0
cmp    eax,DWORD PTR ds:0x0
je     89 <ret_from_sys_call+0x41>
cmp    WORD PTR [esp+0x24],0xf
jne    89 <ret_from_sys_call+0x41>
cmp    WORD PTR [esp+0x30],0x17
jne    89 <ret_from_sys_call+0x41>
mov    ebx,DWORD PTR [eax+0xc]
mov    ecx,DWORD PTR [eax+0x210]
not    ecx
and    ecx,ebx
bsf    ecx,ecx
je     89 <ret_from_sys_call+0x41>
btr    ebx,ecx
mov    DWORD PTR [eax+0xc],ebx
inc    ecx
push   ecx
call   80 <ret_from_sys_call+0x38>
pop    ecx
test   eax,eax
jne    38 <system_call+0x28>
pop    eax
pop    ebx
pop    ecx
pop    edx
add    esp,0x4
pop    fs
pop    es
pop    ds
iret
```

在 sys_call 中根据 eax 找到 sys_call_table 中的函数指针, 进行调用

sizeof(int) 结果是 4, 也就是 dw
sizeof(last_pid) 结果也是 4

## fork

1. fork 首先会遍历 task 数组, 用以防止 pid 重复, 第一次 fork 的时候 task[0] 是有值的
task[0] = 0x24ba0, 这个 task[0] 指针指向的是静态区, 是预先分配好的

2. 获取到 pid 后 调用 copy_process 复制进程内存

3. copy_process 调用 get_free_page 获得一个空闲的 page


4. fork 的流程

- int 0x80

5. fork 是如何做到调用一次返回两次的


## memory management

get_free_page

## process

## file system

