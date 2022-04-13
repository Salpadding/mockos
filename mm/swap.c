#include <linux/mm.h>

/*
 * Get physical address of first (actually last :-) free page, and mark it
 * used. If no free pages left, return 0.
 */
/*
 * 获取首个(实际上是最后1个:-)空闲页面，并标志为已使用。如果没有空闲页面，就返回0。
 */
unsigned long get_free_page(void) {
    int i = PAGING_PAGES - 1;
    int j;
    int *m;

    while(mem_map[i] && i >= 0) {
        i--;
    }
    mem_map[i] = 1;
    i = i * PAGE_SIZE + LOW_MEM;
    m = (void*)(i + 4092);

    for(j = 1024; j >= 1; j--) {
        *m = 0;
        m--;
    }
    return i;
}

