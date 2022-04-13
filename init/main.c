#include <asm/system.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <unistd.h>

int fork(void) {
  long __res;
  __asm__ volatile("int $0x80" : "=a"(__res) : "0"(__NR_fork));
  if (__res >= 0) {
    asm_mark("$0xcafe0001");
    return (int)__res;
  }
  errno = -__res;
  asm_mark("$0xcafe0002");
  return -1;
}

extern void __switch_to(void);
extern void ignore_int(void);
extern void *idt_ptrs[256];

int main(void) {
  printm("================== kernel ==================\n");
  printm("address of fork = %p\n", &fork);
  printm("address of system call = %p\n", &system_call);
  mem_init(4 << 20, 16 << 20);
  printm("address of main = %p\n", &main);
  printm("address of timer interrupt = %p\n", &timer_interrupt);
  printm("address of log_buf = %p\n", &log_buf);
  printm("address of __switch_to = %p\n", &__switch_to);
  printm("address of ignore_int = %p\n", &ignore_int);
  trap_init();
  sched_init();
  sti();
  // swith to ring 3
  // however, the selector '0x17' select the ldt other than gdt
  move_to_user_mode();
  printm("move to user mode success\n");
  // try to fork
  int pid = fork();
  asm_mark("$0xcafe0003");
  printm("return from fork() pid = %d\n", pid);
  asmbp();
  return 0;
}
