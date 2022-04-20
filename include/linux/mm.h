#ifndef _MM_H
#define _MM_H

#define PAGE_SIZE 4096                   /* 定义页面大小(字节数) */
#define LOW_MEM 0x100000                 /* 物理内存地址低端1MB */
#define USED 100                         /* 物理内存被占用 */
#define PAGING_MEMORY (15 * 1024 * 1024) /* 可分页的物理内存大小 */
#define PAGING_PAGES (PAGING_MEMORY >> 12) /* 可分页的物理内存的页面数 */
#define MAP_NR(addr)                                                           \
  (((addr)-LOW_MEM) >> 12) /* 将物理内存地址映射成物理内存页面号 */

extern unsigned char mem_map[PAGING_PAGES];
extern unsigned long HIGH_MEMORY;
void mem_init(long start_mem, long end_mem);
extern unsigned long get_free_page(void);

extern void do_wp_page(unsigned long error_code, unsigned long address);
#define invalidate() __asm__("movl %%eax,%%cr3" ::"a"(0))

#define lin2phy(address) ((unsigned long *) \
		(((address >> 10) & 0xffc) + (0xfffff000 & \
		*((unsigned long *) ((address >> 20) & 0xffc))))) 
#endif
