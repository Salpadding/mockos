#ifndef SYSTEM_H
#define SYSTEM_H
#include <linux/head.h>

/* 利用iret指令实现从内核模式移到用户模式去执行初始任务0 */
#define move_to_user_mode()						\
__asm__ (										\
	"movl %%esp,%%eax\n\t"						\
	"pushl $0x17\n\t"							\
	"pushl %%eax\n\t"							\
	"pushfl\n\t"								\
	"pushl $0x0f\n\t"							\
	"pushl $1f\n\t"								\
	"iret\n"									\
"1:\tmovl $0x17,%%eax\n\t"						\
	"mov %%ax,%%ds\n\t"							\
	"mov %%ax,%%es\n\t"							\
	"mov %%ax,%%fs\n\t"							\
	"mov %%ax,%%gs"								\
	:::"ax")

#define asmbp() __asm__("1:\tnop \n\t" "jmp 1b\n\t")
#define asm_mark(M) asm volatile("pushl %eax\n\t" "movl " M ", %eax\n\t" "popl %eax\n\t")

#define sti() __asm__ ("sti"::)			/* 开中断 */
#define cli() __asm__ ("cli"::)			/* 关中断 */
#define nop() __asm__ ("nop"::)			/* 空操作 */

/**
 * 设置门描述符宏
 * @param[in]	gate_addr	在中断描述符表中的偏移量
 * @param[in]	type		门描述符类型
 * @param[in]	dpl			特权级信息
 * @param[in]	addr		中断或异常过程函数地址
 */
#define _set_gate(gate_addr, type, dpl, addr)				\
	__asm__ ("movw %%dx,%%ax\n\t"							\
			"movw %0,%%dx\n\t"								\
			"movl %%eax,%1\n\t"								\
			"movl %%edx,%2"									\
			:												\
			: "i" ((short) (0x8000+(dpl<<13)+(type<<8))),	\
			"o" (*((char *) (gate_addr))),					\
			"o" (*(4+(char *) (gate_addr))),				\
			"d" ((char *) (addr)),"a" (0x00080000))


/**
 * 在全局表中设置任务状态段/局部表描述符
 * 状态段局部表段的长度均被设置成104字节。%0 - eax(地址addr)；%1 - (描述符项n的地址); %2 - (描述
 * 符项n的地址偏移2处)；%3 - (描述符项n的地址偏移4处); %4 - (描述符项n的地址偏移5处);%5 - (描述
 * 符项n的地址偏移6处);%6 - (描述符项n的地址偏移7处);
 * @param[in]	n		在全局表中描述符项n所对应的地址
 * @param[in]	addr	状态段/局部表所在内存的基地址
 * @param[in]	type	描述符中的标志类型字节
 */
// mov eax,addr
// mov word [n], 0x68
// mov [n + 2], ax
// ror eax, 0x10
// mov [n + 4], al
// mov byte [n + 5], 0x89
// mov byte [n + 6], 0
// mov [n + 7], ah
// ror eax, 0x10
// n = &gdt[4], addr = &(init_task.task.tss)
// char* p = (void*)(&gdt[4]);
// 0x0000890250300068
// limit = 104, base0 = 0x25030, limit 表示 tss 长度, tss 的 access byte 要打开 present bit 和 executable bit
// 这里的 0x89 把 access bit 也打开了, base0 是 addr 也就是 &(init_task.task.tss)
#define _set_tssldt_desc(n,addr,type)								\
__asm__ (															\
	"movw $104,%1\n\t"												\
	"movw %%ax,%2\n\t"												\
	"rorl $16,%%eax\n\t"											\
	"movb %%al,%3\n\t"												\
	"movb $" type ",%4\n\t"											\
	"movb $0x00,%5\n\t"												\
	"movb %%ah,%6\n\t"												\
	"rorl $16,%%eax"												\
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)),			\
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7))						\
	)

/** 
 * 设置陷阱门函数
 * @param[in]	n		中断号
 * param[in]	addr	中断程序偏移地址
 */
#define set_trap_gate(n, addr)		_set_gate(&idt[n], 15, 0, addr)

/**
 * 设置系统陷阱门函数
 * @param[in]	n		中断号
 * @param[in]	addr	中断程序偏移直
 */
#define set_system_gate(n, addr) 	_set_gate(&idt[n], 15, 3, addr)

/** 
 * 设置中断门函数(自动屏蔽随后的中断)
 * @param[in]	n		中断号
 * @param[in]	addr	中断程序偏移地址
 */
#define set_intr_gate(n, addr)		_set_gate(&idt[n], 14, 0, addr)

/**
 * 在全局表中设置任务状态段描述符(任务状态段描述符的类型是0x89)
 * @param[in]	n		该描述符的指针
 * @param[in]	addr	描述符项中段的基地址值
 */
#define set_tss_desc(n,addr)	_set_tssldt_desc(((char *) (n)),addr, "0x89")

/**
 * 在全局表中设置局部表描述符(局部表段描述符的类型是0x82)
 * @param[in]	n		该描述符的指针
 * @param[in]	addr	描述符项中段的基地址值
 */
#define set_ldt_desc(n, addr)	_set_tssldt_desc(((char *) (n)),addr, "0x82")

#define set_system_gate(n, addr) 	_set_gate(&idt[n], 15, 3, addr)

#define use_sys_ds()						\
__asm__ (										\
"1:\tmovl $0x1b,%eax\n\t"						\
	"mov %ax,%ds\n\t"							\
)


#define use_user_ds()						\
__asm__ (										\
"1:\tmovl $0x17,%eax\n\t"						\
	"mov %ax,%ds\n\t"							\
)
#endif
