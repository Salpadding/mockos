#include <asm/io.h>
#include <asm/system.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sys.h>
#include <unistd.h>

#define MAIN_MEM_START 4 << 20
#define MAIN_MEM_END 16 << 20
#define BUF_MEM_END 4 * 1024 * 1024

_syscall0(int, fork);
_syscall0(int, kdebug);
_syscall1(int, klock, int, n);
_syscall1(int, kunlock, int, n);

extern void __switch_to(void);
extern void ignore_int(void);
extern void *idt_ptrs[256];
extern int init_serial();
extern void hd_init(void);
extern void buffer_init(long buffer_end);

static int cnt;

extern int end;

int main(void) {
    init_serial();
    printm("printk from kernel space end = 0x%x\n", &end);
    mem_init(4 << 20, 16 << 20);
    buffer_init(BUF_MEM_END);
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
