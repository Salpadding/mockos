#include <asm/system.h>
#include <linux/kernel.h>
#include <linux/mm.h>

#define copy_page(from, to)                                                    \
  __asm__("cld ; rep ; movsl" ::"S"(from), "D"(to), "c"(1024))

unsigned long HIGH_MEMORY = 0;

unsigned char mem_map[PAGING_PAGES] = {
    0,
};

void mem_init(long start_mem, long end_mem) {
  printm("start_mem = %d\nend_mem %d\n", start_mem, end_mem);
  HIGH_MEMORY = end_mem;
  int i;
  for (i = 0; i < PAGING_PAGES; i++) {
    mem_map[i] = USED;
  }
  i = MAP_NR(start_mem);
  end_mem -= start_mem;
  end_mem >>= 12;

  while (end_mem-- > 0) {
    mem_map[i++] = 0;
  }
}

int copy_page_tables(unsigned long from, unsigned long to, long size) {
  unsigned long *from_page_table;
  unsigned long *to_page_table;
  unsigned long this_page;
  unsigned long *from_dir, *to_dir;
  unsigned long new_page;
  unsigned long nr;

  // linux0.12 内核只有一个页表, 利用 ldt 的 base 实现各个进程间内存的互相隔离
  // 这里的工作实际上是进行内存拷贝, 但是只拷贝页表
  // 一个 pde 对应的内存大小是 4k * 1024 = 4M

  /* 源地址和目的地址都需要在4MB内存边界地址 */
  if ((from & 0x3fffff) || (to & 0x3fffff)) {
    panic("copy_page_tables called with wrong alignment");
  }
  /* 源地址的目录项指针，目标地址的目录项指针， 需要复制的目录项数 */
  // index of page directory  (x & 0xffc00000) >> 20, 其实相当于 x >> 22
  // index of page table (x & 0x3ff000) >> 12
  from_dir = (unsigned long *)((from >> 20) & 0xffc); /* _pg_dir = 0 */

  to_dir = (unsigned long *)((to >> 20) & 0xffc);
  size = ((unsigned)(size + 0x3fffff)) >> 22; // dividend = 4m, 向上取整

  /* 开始页表项复制操作 */
  for (; size-- > 0; from_dir++, to_dir++) {
    if (1 & *to_dir) {
      panic("copy_page_tables: already exist");
    }
    if (!(1 & *from_dir)) {
      continue;
    }
    // *from_dir, dereference the page dir entry pointer, and drop options
    from_page_table = (unsigned long *)(0xfffff000 & *from_dir);

    // allocate a free physical page
    if (!(to_page_table = (unsigned long *)get_free_page())) {
      return -1; /* Out of memory, see freeing */
    }
    // setup page table
    *to_dir = ((unsigned long)to_page_table) | 7;
    /* 源地址在内核空间，则仅需复制前160页对应的页表项(nr = 160)，对应640KB内存
     */
    nr = (from == 0) ? 0xA0 : 1024;
    /* 循环复制当前页表的nr个内存页面表项 */
    for (; nr-- > 0; from_page_table++, to_page_table++) {
      this_page = *from_page_table;
      if (!this_page) {
        continue;
      }
      this_page &=
          ~2; /* 让页表项对应的内存页面只读 这也是 copy on write 技术实现关键 */
      *to_page_table = this_page;
      /* 物理页面的地址在1MB以上，则需在mem_map[]中增加对应页面的引用次数 */
      if (this_page > LOW_MEM) {
        *from_page_table = this_page; /* 令源页表项也只读 */
        this_page -= LOW_MEM;
        this_page >>= 12;
        mem_map[this_page]++;
      }
    }
  }
  invalidate();
  return 0;
}

void un_wp_page(unsigned long *table_entry);

void do_wp_page(unsigned long error_code, unsigned long address) {
  // lookup page table in page directory
  // no need to dereference by long * pd = 0;  *x = pd[(address >> 22)]
  // since the address of pd is zero 
  unsigned long *x = (unsigned long *)((address >> 20) & 0xffc);
  unsigned long *y = (void *)(*x & 0xfffff000);

  // y is address of page directory
  unsigned long *z = &(y[(address >> 12) & 0x3ff]);
  un_wp_page(z);
}

void un_wp_page(unsigned long *table_entry) {
  unsigned long old_page, new_page;

  old_page = 0xfffff000 & *table_entry;

  /* 即如果该内存页面此时只被一个进程使用，就直接把属性改为可写即可 */
  if (old_page >= LOW_MEM && mem_map[MAP_NR(old_page)] == 1) {
    *table_entry |= 2;
    invalidate();
    return;
  }

  /* 申请一页空闲页面给执行写操作的进程单独使用，取消页面共享。复制原页面的内容至新页面，
  将指定页表项值更新为新页面地址 */
  new_page = get_free_page();
  if (old_page >= LOW_MEM)
    mem_map[MAP_NR(old_page)]--;
  copy_page(old_page, new_page);
  *table_entry = new_page | 7;
  invalidate();
}
