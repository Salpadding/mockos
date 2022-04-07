#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <unistd.h>


_syscall0(int, fork)

int main(void) {
  printm("============== KERNEL ============\n");
  printm("address of fork = %p\n", &fork);
  printm("address of system call = %p\n", &system_call);
  mem_init(4 << 20, 16 << 20);
  printm("address of main = %p\n", &main);
  printm("address of timer interrupt = %p\n", &timer_interrupt);

  sched_init();
  sti();
  // swith to ring 3
  // however, the selector '0x17' select the ldt other than gdt
  // move_to_user_mode();
  // try to fork
  fork();
  while (1) {
    __asm__("hlt");
  }
  return 0;
}
