```sh
sudo apt install gcc
sudo apt install make
sudo apt install gcc-multilib
```

## 断点调试

1. setup.S  0x90020
2. setup.S 跳到 head.s 0x9030f
3. 调试发现第一个收到的来自硬件的中断是 0x20, 也就是时钟


## 通过调试解决了哪些问题?

1. head.s 中 `lss stack_start, %esp` 的作用?

在 c 语言中分配了一块内存, 大小是 4kb, 这个指令重新定义了栈区
如果不这么做的话栈区是未定义的, 可能会产生段错误

2. c 语言入参顺序和 栈指针从高向低增长有什么联系?

c 语言左边的参数是低地址, 右边的参数是高地址, 而随着栈的增长, 栈指针会向低地址移动
当汇编中调用 c 语言的函数时, 利用栈传参, 那么后入栈的参数就位于低地址也就是靠左的入参

3. 如何进入用户态? 进入用户态前为什么要设置 ldt 和 tss? 

使用 iret 进入用户态, 用户态位于最低的特权级



4. 如何进入内核态? 进入内核态后栈上备份了哪些寄存器? ss 和 esp 从哪里加载?
5. lss 指令和 mov 指令的区别?
6. segment selector 什么时候 select ldt?
7. je, jne, jz, jnz

je, jz: ZF flag 为 true 时候跳转
jne, jnz ZF flag 为 false 时候跳转

8. test 指令是把两个寄存器作 and, 然后放到 zero flag 里面, `test eax eax` `js error` 当 eax < 0 时候跳转
9. `get_base` 用 c 语言表示如下

```c
unsigned long get_base_c(void *p) {
    unsigned long* pt = p;
    unsigned long a = pt[0];
    unsigned long b = pt[1];
    return (a >> 16) | ((b & 0xff) << 16) | (b & 0xff000000);
}
```

10. `get_limit`

```asm
mov eax, 0x0f       
lsl edi, ax      
inc edi                   
```

这里用到了一个特殊的指令 load segment limit(lsl), 把 ax 中指定的段的 limit 取出来放到 edi 里面

11.  `set_base` 用 c 语言表示

```c
void set_base_c(void *p, unsigned long base) {
    unsigned long* a = p;
    unsigned long* b = &a[1];
    *a = (*a & 0x0000ffff) | ((base & 0xffff) << 16);
    *b = (*b & 0xffffff00) | ((base & 0xff0000) >> 16);
    *b = (*b & 0x00ffffff) | (base & 0xff000000);
}
```

12. logical address, linear address, physical address

logical address 是类似 `ds:[eax]` 这种表示, logical address 需要去查 ds 对应的 segment descriptor
然后加上 segment descriptor 里面的 offset 变成 linear address

linear address 取高 10 位去 page directory 里查到 page table 地址
再继续取后面 10 位在 page table 查到 physical address 然后再加上 linear address 里的低 12 位就得到最终的 physical address

13. `std; repne; scasb` 如何理解

repne + scasb 是字符串比较指令, repeat while not equal + scan string byte by byte
受影响的寄存器有 `ecx` 和 `es:[edi]`, ax 或 al 是比较的值

The SCAS instruction subtracts the destination string element from the contents of the EAX, AX, or AL register (depending on operand length) and updates the status flags according to the results. The string element and register contents are not modified. The following “short forms” of the SCAS instruction specify the operand length: SCASB (scan byte string), SCASW (scan word string), and SCASD (scan doubleword string).

14. `rep movsb` 如何理解

从 ds:esi 往 es:edi 逐字节的拷贝数据, 拷贝的数据量由 ecx 指定

15. 为什么我在 gcc 内联汇编里定义全局符号提示我重复的符号？

我在调试 gcc 内联汇编代码的时候想插入一些符号用于打断点, 然后编译的时候提示我符号冲突
gcc 会对函数调用作优化, 对函数进行内联, 所以汇编代码可能会被内联到多个地方去, 导致符号冲突

所以在用到汇编的函数的地方可以禁用 gcc 内联优化

```c
void __attribute__((noinline)) infinite_loop(void) {
    __asm__ __volatile__ (".global inf_lp\n\t" "inf_lp:\n\t" "jmp inf_lp\n\t");
}
```

16. linux0.12 如何实现任务切换?

```c
#define switch_to(n) {							\
struct {long a,b;} __tmp; 						\
__asm__("cmpl %%ecx,current\n\t"			 	\
	"je 1f\n\t" 								\
    ".global __switch_to\n\t" \
    "__switch_to:\n\t"                           \
	"movw %%dx,%1\n\t" 							\
	"xchgl %%ecx,current\n\t" 					\
	"ljmp *%0\n\t" 								\
	"cmpl %%ecx,last_task_used_math\n\t" 		\
	"jne 1f\n\t" 								\
	"clts\n"									\
	"1:" 										\
	::"m" (*&__tmp.a),"m" (*&__tmp.b), 			\
	"d" (_TSS(n)),"c" ((long) task[n])); 		\
}
```
[这里](https://stackoverflow.com/questions/33783692/what-does-the-ljmp-instruction-do-in-the-linux-kernel-fork-system-call) 解释了 jmpf 的两种形式

此处的 jmpf 是通过内存寻址而不是寄存器, a 是 offset 而 b 的低16位是 segment

调试时候发现一条 `jmp ss:[esp]` 指令, 注意这里 b 指向的是 task gate, 所以这里的 a 实际上会被忽略, linux 0.12 是利用 tss 实现的任务切换

在 jmpf 的时候如果 segment 指向了 gdt 里面的 tss 就会发生下面的事情:

切换前:

寄存器上下文:

```
rax: 00000000_00000001
rbx: 00000000_00000001
rcx: 00000000_00000001
rdx: 00000000_00000001
rsp: 00000000_00fffee8
rbp: 00000000_00000001
rsi: 00000000_00000001
rdi: 00000000_00000001
r8 : 00000000_00000000
r9 : 00000000_00000000
r10: 00000000_00000000
r11: 00000000_00000000
r12: 00000000_00000000
r13: 00000000_00000000
r14: 00000000_00000000
r15: 00000000_00000000
rip: 00000000_00006f1d
eflags 0x00000006: id vip vif ac vm rf nt IOPL=0 of df if tf sf zf af PF cf
es:0x0010, dh=0x00c09300, dl=0x00000fff, valid=7
        Data segment, base=0x00000000, limit=0x00ffffff, Read/Write, Accessed
cs:0x0008, dh=0x00c09b00, dl=0x00000fff, valid=1
        Code segment, base=0x00000000, limit=0x00ffffff, Execute/Read, Non-Conforming, Accessed, 32-bit
ss:0x0010, dh=0x00c09300, dl=0x00000fff, valid=7
        Data segment, base=0x00000000, limit=0x00ffffff, Read/Write, Accessed
ds:0x0010, dh=0x00c09300, dl=0x00000fff, valid=7
        Data segment, base=0x00000000, limit=0x00ffffff, Read/Write, Accessed
fs:0x0017, dh=0x04c0f300, dl=0x0000009f, valid=1
        Data segment, base=0x04000000, limit=0x0009ffff, Read/Write, Accessed
gs:0x0017, dh=0x04c0f300, dl=0x0000009f, valid=1
        Data segment, base=0x04000000, limit=0x0009ffff, Read/Write, Accessed
ldtr:0x0038, dh=0x000082ff, dl=0xf3980068, valid=1
tr:0x0030, dh=0x00008bff, dl=0xf3b00068, valid=1
gdtr:base=0x0000000000005cc0, limit=0x7ff
idtr:base=0x00000000000054c0, limit=0x7ff
```

切换发起任务的 tss:

```
back_link = 0x0 esp0 = 0x1000000 ss0 = 0x10 esp1 = 0x0 ss1 = 0x0 esp2 = 0x0 ss2 = 0x0 cr3 = 0x0 eip = 0x6f20 eflags = 0x6 eax = 0xfff000 ecx = 0xfff000 edx = 0x20 ebx = 0xffffffff esp = 0xffff48 ebp = 0x3856c esi = 0x0 edi = 0xffc es = 0x10 cs = 0x8 ss = 0x10 ds = 0x10 fs = 0x17 gs = 0x17 ldt = 0x38 trace_bitmap = 0x80000000
```

切换至任务的 tss:

```
back_link = 0x0 esp0 = 0x26400 ss0 = 0x10 esp1 = 0x0 ss1 = 0x0 esp2 = 0x0 ss2 = 0x0 cr3 = 0x0 eip = 0x6f20 eflags = 0x87 eax = 0xfff000 ecx = 0x25400 edx = 0x30 ebx = 0xf esp = 0x26390 ebp = 0x3 esi = 0x1 edi = 0xffc es = 0x10 cs = 0x8 ss = 0x10 ds = 0x10 fs = 0x17 gs = 0x17 ldt = 0x28 trace_bitmap = 0x80000000
```

切换后:

寄存器上下文:

```
rax: 00000000_00fff000
rbx: 00000000_0000000f
rcx: 00000000_00025400
rdx: 00000000_00000030
rsp: 00000000_00026390
rbp: 00000000_00000003
rsi: 00000000_00000001
rdi: 00000000_00000ffc
r8 : 00000000_00000000
r9 : 00000000_00000000
r10: 00000000_00000000
r11: 00000000_00000000
r12: 00000000_00000000
r13: 00000000_00000000
r14: 00000000_00000000
r15: 00000000_00000000
rip: 00000000_00006f20
eflags 0x00000087: id vip vif ac vm rf nt IOPL=0 of df if tf SF zf af PF CF
es:0x0010, dh=0x00c09300, dl=0x00000fff, valid=1
        Data segment, base=0x00000000, limit=0x00ffffff, Read/Write, Accessed
cs:0x0008, dh=0x00c09b00, dl=0x00000fff, valid=1
        Code segment, base=0x00000000, limit=0x00ffffff, Execute/Read, Non-Conforming, Accessed, 32-bit
ss:0x0010, dh=0x00c09300, dl=0x00000fff, valid=1
        Data segment, base=0x00000000, limit=0x00ffffff, Read/Write, Accessed
ds:0x0010, dh=0x00c09300, dl=0x00000fff, valid=1
        Data segment, base=0x00000000, limit=0x00ffffff, Read/Write, Accessed
fs:0x0017, dh=0x00c0f300, dl=0x0000009f, valid=1
        Data segment, base=0x00000000, limit=0x0009ffff, Read/Write, Accessed
gs:0x0017, dh=0x00c0f300, dl=0x0000009f, valid=1
        Data segment, base=0x00000000, limit=0x0009ffff, Read/Write, Accessed
ldtr:0x0028, dh=0x00008202, dl=0x57980068, valid=1
tr:0x0020, dh=0x00008b02, dl=0x57b00068, valid=1
gdtr:base=0x0000000000005cc0, limit=0x7ff
idtr:base=0x00000000000054c0, limit=0x7ff
```

切换发起任务的 tss:

```
back_link = 0x0 esp0 = 0x1000000 ss0 = 0x10 esp1 = 0x0 ss1 = 0x0 esp2 = 0x0 ss2 = 0x0 cr3 = 0x0 eip = 0x6f20 eflags = 0x6 eax = 0x1 ecx = 0x1 edx = 0x1 ebx = 0x1 esp = 0xfffee8 ebp = 0x1 esi = 0x1 edi = 0x1 es = 0x10 cs = 0x8 ss = 0x10 ds = 0x10 fs = 0x17 gs = 0x17 ldt = 0x38 trace_bitmap = 0x80000000
```

切换至任务的 tss:

```
back_link = 0x0 esp0 = 0x26400 ss0 = 0x10 esp1 = 0x0 ss1 = 0x0 esp2 = 0x0 ss2 = 0x0 cr3 = 0x0 eip = 0x6f20 eflags = 0x87 eax = 0xfff000 ecx = 0x25400 edx = 0x30 ebx = 0xf esp = 0x26390 ebp = 0x3 esi = 0x1 edi = 0xffc es = 0x10 cs = 0x8 ss = 0x10 ds = 0x10 fs = 0x17 gs = 0x17 ldt = 0x28 trace_bitmap = 0x80000000
```

由此可以得知切换任务的时候, 原先的任务的上下文会保存到 tss 中, 然后切换到新的 tss

17. 为什么清除 nt

```c
  /* Clear NT, so that we won't have troubles with that later on */
__asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
```

清除 nt 的目的是消除未定义的行为, 防止 iretd 出现没有预料到的行为, 当 nt 为 1 的时候 iretd 的行为和 tss 有关

18. 为什么要给 task 添加 state 字段

防止 tss 没有初始化完毕就被调度, 产生未定义行为 

19. 如何理解 fork 系统调用, fork 复制出来的进程的状态对应到父进程的什么代码?

20. 如何理解 linux 0.12 的写时复制技术?

21. 使用写时复制技术进行 fork 的话, 栈区要提前复制吗?

需要提前复制的,如果父进程在新进程之前被调度了,那么新进程的栈区就被破坏了,可能产生未定义的行为。
linux 0.12 的 fork 机制不够完善, 没有预先拷贝栈内存, 所以我在实现 fork 的时候把中断暂时关闭然后在 copy_process 成功后立刻调度新进程


## 遗留问题

1. 在使用cli 关闭中断的情况下进入用户态，然后用户态进程调用 int 0x80 会产生什么结果?
2. 为什么在 `timer_interrupt` 中要发送 EOI 给中断控制器?
3. 调试发现设置 `time_interrupt` 后要用 outb 指令作 io, 后续使用 hlt 指令才可以被中断唤醒

从 0x21 端口读一个字节, 读到的是 0xff, 与上 0xfe, ax 剩下的是 0xfe, 然后向 0x21 端口写入 0xfe

```asm
mov edx, 0x21       
in al, dx                 
and eax, 0xfe       
out dx, al                
```

这里其实是在对中断芯片进行编程
