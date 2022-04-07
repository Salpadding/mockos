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

1. setup.s 中 `lss stack_start, %esp` 的作用?
2. c 语言入参顺序和 栈指针从高向低增长有什么联系?
3. 如何进入用户态? 进入用户态前为什么要设置 ldt 和 tss? 
4. 如何进入内核态? 进入内核态后栈上备份了哪些寄存器? ss 和 esp 从哪里加载?
5. lss 指令和 mov 指令的区别?
6. segment selector 什么时候 select ldt?
7. je, jne, jz, jnz

je, jz: ZF flag 为 true 时候跳转
jne, jnz ZF flag 为 false 时候跳转


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
