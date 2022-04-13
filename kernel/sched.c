#include <asm/io.h>
#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>

unsigned long volatile jiffies = 0;

struct task_struct *last_task_used_math = NULL; /* 上一个使用过协处理器的进程 */

// however, the so called user_stack below is utilized by kernel
long user_stack[PAGE_SIZE >> 2];

union task_union {
  struct task_struct task;
  char stack[PAGE_SIZE];
};

static union task_union init_task = {
    INIT_TASK,
};
struct task_struct *current = &(init_task.task); /* 当前任务指针 */

struct task_struct *task[NR_TASKS] = {
    &(init_task.task),
};

struct {
  long *a;
  short b;
  // the address of stack pointer decrease when growing
} stack_start = {&user_stack[PAGE_SIZE >> 2], 0x10};

unsigned long get_base(void *p) {
  unsigned long *pt = p;
  unsigned long a = pt[0];
  unsigned long b = pt[1];
  return (a >> 16) | ((b & 0xff) << 16) | (b & 0xff000000);
}

void set_base(void *p, unsigned long base) {
  unsigned long *a = p;
  unsigned long *b = &a[1];
  *a = (*a & 0x0000ffff) | ((base & 0xffff) << 16);
  *b = (*b & 0xffffff00) | ((base & 0xff0000) >> 16);
  *b = (*b & 0x00ffffff) | (base & 0xff000000);
}

/* 内核调度程序的初始化子程序 */
void sched_init(void) {
  int i;
  struct desc_struct *p; /* 描述符表结构指针 */

  /* 这个判断语句并无必要 */
  if (sizeof(struct sigaction) != 16) {
    panic("Struct sigaction MUST be 16 bytes");
  }
  set_tss_desc(gdt + FIRST_TSS_ENTRY, &(init_task.task.tss));
  set_ldt_desc(gdt + FIRST_LDT_ENTRY, &(init_task.task.ldt));
  p = gdt + 2 + FIRST_TSS_ENTRY;
  for (i = 1; i < NR_TASKS; i++) {
    task[i] = NULL;
    p->a = p->b = 0;
    p++;
    p->a = p->b = 0;
    p++;
  }
  /* Clear NT, so that we won't have troubles with that later on */
  __asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
  ltr(0);
  lldt(0);
  set_intr_gate(0x20, &timer_interrupt);
  outb(inb_p(0x21) & ~0x01, 0x21);
  /* 设置系统调用的系统陷阱 */
  set_system_gate(0x80, &system_call);
}

void schedule(){};

void switch_to_c(int nr) {
    switch_to("__switch_to_c", nr);
}

void __attribute__((noinline)) do_timer(long cpl) {
  // a simple scheduler, always switch to the first
  int i;
  for (i = 0; i < NR_TASKS; i++) {
    if (task[i] != NULL && task[i]->pid != current->pid && task[i]->state == TASK_RUNNING) {
      switch_to("__switch_to", i);
      return;
    }
  }
}
