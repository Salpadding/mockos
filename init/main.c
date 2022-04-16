#include <asm/system.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sys.h>
#include <unistd.h>

_syscall0(int, fork);

extern void __switch_to(void);
extern void ignore_int(void);
extern void *idt_ptrs[256];
extern int init_serial();

int main(void) {
  init_serial();
  printk("printk from kernel space\n");
  mem_init(4 << 20, 16 << 20);
  trap_init();
  sched_init();
  sti();
  // swith to ring 3
  // however, the selector '0x17' select the ldt other than gdt
  move_to_user_mode();
  // printk("move to user mode success\n");
  //  try to fork
  printk("printk from user space\n");
  int pid = fork();
  // printk("pid = %d\n", pid);
  // printm("return from fork() pid = %d\n", pid);

  /* 调度函数发现系统中没有其他程序可以运行就会切换到任务0 */
  asmbp();
  return 0;
}
