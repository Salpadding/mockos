#include <asm/system.h>
#include <linux/kernel.h>

void page_fault(void); // int14(mm/page.s)

extern void *idt_ptrs[256];

/**
 * 异常(陷阱)中断程序初始化
 * 设置它们的中断调用门(中断向量)。set_trap_gate()与set_system_gate()都使用了中断描述符表IDT中
 * 的陷阱门(Trap
 * Gate)，它们之间的主要区别在于前者设置的特权级为0，后者是3。因此断点陷阱中断int3，
 * 溢出中断overflow和边界出错中断bounds可以由任何程序调用。
 */
void trap_init(void) {
  int i = 0;
  for (; i < 256; i++) {
    unsigned long off = (unsigned long)idt_ptrs[i];
    idt[i].a = (idt[i].a & (~0xffffL)) | (off & 0xffffL);
  }
  set_trap_gate(14, &page_fault);
}
