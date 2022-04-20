#include <asm/system.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sys.h>
#include <unistd.h>

_syscall0(int, fork);
_syscall0(int, kdebug);
_syscall1(int, klock, int, n);
_syscall1(int, kunlock, int, n);

extern void __switch_to(void);
extern void ignore_int(void);
extern void *idt_ptrs[256];
extern int init_serial();

static int cnt;

int main(void) {
  init_serial();
  printm("printk from kernel space\n");
  mem_init(4 << 20, 16 << 20);
  trap_init();
  sched_init();
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
  printm("sys_debug\n");
  return 0;
}
