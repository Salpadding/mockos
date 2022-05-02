#include <asm/system.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sys.h>
#include <unistd.h>
#include <asm/io.h>

_syscall0(int, fork);
_syscall0(int, kdebug);
_syscall1(int, klock, int, n);
_syscall1(int, kunlock, int, n);

extern void __switch_to(void);
extern void ignore_int(void);
extern void *idt_ptrs[256];
extern int init_serial();
extern void hd_init(void);

static int cnt;

int main(void) {
    init_serial();
    printm("printk from kernel space\n");
    mem_init(4 << 20, 16 << 20);
    trap_init();
    sched_init();
    hd_init();    
    sti();
    // swith to ring 3
    // however, the selector '0x17' select the ldt other than gdt
    move_to_user_mode();
    // printk("move to user mode success\n");
    //  try to fork
    printm("printk from user space\n");
    // asmbp();
    int pid = fork();
    // printk("pid = %d\n", pid);
    printm("return from fork() pid = %d\n", pid);
    printm("hello world\n");
    kdebug();
    asmbp();
    return 0;
}

int sys_kdebug() {
    outb_p(0xc8, 0x3f6); // 每次都 reset 一下 control block
    outb_p(0xff, 0x1f1); // 不清楚 应该和异常处理有关
    outb_p(0x02, 0x1f2); // 写入要读的扇区数量
    // lba address
    outb_p(0x01, 0x1f3);
    outb_p(0, 0x1f4);
    outb_p(0, 0x1f5);
    outb_p(0xa0, 0x1f6);

    // 发出读的指令
    outb(0x20, 0x1f7);

    return 0;
}
