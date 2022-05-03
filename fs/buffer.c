#include <asm/system.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/kernel.h>

/*
 * Ok, this is getblk, and it isn't very clear, again to hinder
 * race-conditions. Most of the code is seldom used, (ie repeating),
 * so it should be much more efficient than it looks.
 *
 * The algoritm is changed: hopefully better, and an elusive bug removed.
 */
/*
 * OK，下面是getblk函数，该函数的逻辑并不是很清晰，同样也是因为要考虑竞争条件问题。其中大部分代
 * 码很少用到(例如重复操作语句)，因此它应该比看上去的样子有效得多。
 *
 * 算法已经作了改变：希望能更好，而且一个难以琢磨的错误已经去除。
 */
#define BADNESS(bh) (((bh)->b_dirt << 1) + (bh)->b_lock)

#define _hashfn(dev, block)                                                    \
    (((unsigned)(dev ^ block)) %                                               \
     NR_HASH) // a simple hash function implemented by xor
#define hash(dev, block) hash_table[_hashfn(dev, block)] // hash table

/* 系统所含缓冲个数 */
int NR_BUFFERS = 0;

// 由编译器生成
extern int end;

struct buffer_head *start_buffer = (void *)(&end);

/* 空闲缓冲块链表头指针 */
static struct buffer_head *free_list;

/* 缓冲区Hash表数组 */
struct buffer_head *hash_table[NR_HASH];

/* 等待空闲缓冲块而睡眠的任务队列 */
static struct task_struct *buffer_wait = NULL;

static inline void wait_on_buffer(struct buffer_head *bh) {
    cli();
    while (bh->b_lock) {
        sleep_on(&bh->b_wait);
    }
    sti();
}

// remove head from hash table and free list
static inline void remove_from_queues(struct buffer_head *bh) {
    // remove from hash table
    if (bh->b_next) {
        bh->b_next->b_prev = bh->b_prev;
    }
    if (bh->b_prev) {
        bh->b_prev->b_next = bh->b_next;
    }
    if (hash(bh->b_dev, bh->b_blocknr) == bh) {
        hash(bh->b_dev, bh->b_blocknr) = bh->b_next;
    }

    // how to use lock-free linked list
    if (!(bh->b_prev_free) || !(bh->b_prev_free)) {
        panic("Free block list corrupted");
    }

    bh->b_next_free->b_prev_free = bh->b_prev_free;
    bh->b_prev_free->b_next_free = bh->b_next_free;

    // the first free buffer head
    if (free_list == bh) {
        free_list = free_list->b_next_free;
    }
}

static inline void insert_into_queues(struct buffer_head *bh) {
    // append to free list
    struct buffer_head *last = free_list->b_prev_free;
    bh->b_prev_free = last;
    bh->b_next_free = free_list;
    last->b_next_free = bh;
    free_list->b_prev_free = bh;

    bh->b_prev = NULL;
    bh->b_next = NULL;

    // skip buffer heads not related to underlying devcie
    if (!bh->b_dev) {
        return;
    }

    // insert into hash table
    struct buffer_head *first = hash(bh->b_dev, bh->b_blocknr);
    hash(bh->b_dev, bh->b_blocknr) = bh;
    bh->b_next = first;
    if (first) {
        first->b_prev = bh;
    }
}

// lookup buffer in hash table
static struct buffer_head *find_buffer(int dev, int block) {
    struct buffer_head *tmp;
    for (tmp = hash(dev, block); tmp != NULL; tmp = tmp->b_next) {
        if (tmp->b_dev == dev && tmp->b_blocknr == block) {
            return tmp;
        }
    }
    return NULL;
}

// 利用 hash table 在高速缓冲区中寻找指定的缓冲块
struct buffer_head *get_hash_table(int dev, int block) {
    struct buffer_head *bh;

    for (;;) {
        if (!(bh = find_buffer(dev, block))) {
            return NULL;
        }

        bh->b_count++;
        wait_on_buffer(bh);

        // 为什么这里又校验了一次?
        if (bh->b_dev == dev && bh->b_blocknr == block) {
            return bh;
        }
        bh->b_count--;
    }
}

// 被占用的 buffer head 会放在靠后的地方
struct buffer_head *getblk(int dev, int block) {
    struct buffer_head *tmp, *bh;

repeat:
    // 尝试从 hash table 获取
    // hash table 中可能有 dirt 或者 locked 的 buffer 吗
    bh = get_hash_table(dev, block);

    if (bh) {
        return bh;
    }

    tmp = free_list;
    do {
        if (tmp->b_count) {
            // skip used buffer
            continue;
        }
        // 跳过 dirt 的 block
        // badness = not dirt & not lock < dirt & not lock = not dit & lock <
        // dirt + lock
        if (!bh || BADNESS(tmp) < BADNESS(bh)) {
            bh = tmp;
            if (!BADNESS(tmp)) {
                break;
            }
        }
    } while ((tmp = tmp->b_next_free) != free_list);

    if (!bh) {
        sleep_on(&buffer_wait);
        goto repeat;
    }

    wait_on_buffer(bh);
    if (bh->b_count) {
        goto repeat;
    }

    while (bh->b_dirt) {
        goto repeat;
    }

    if (find_buffer(dev, block)) {
        goto repeat;
    }

    bh->b_count = 1;
    bh->b_dirt = 0;
    bh->b_uptodate = 0;

    remove_from_queues(bh);
    bh->b_dev = dev;
    bh->b_blocknr = block;
    insert_into_queues(bh);
    return bh;
}


int sys_kdebug() {
    struct buffer_head* bh = getblk(0x300, 0);
    printm("getblk() returned success\n");
}


void buffer_init(long buffer_end) {
    struct buffer_head *h = start_buffer;
    void *b;
    int i;

    if (buffer_end == 1 << 20) {
        // 因为内存足够大 这条分支实际上没走到
        b = (void *)(640 * 1024);
    } else {
        b = (void *)(buffer_end);
    }

    while ((b -= BLOCK_SIZE) >= ((void *)(h + 1))) {
        h->b_dev = 0;
        h->b_dirt = 0;
        h->b_count = 0;
        h->b_lock = 0;
        h->b_uptodate = 0;
        h->b_wait = NULL;
        h->b_next = NULL;
        h->b_prev = NULL;
        h->b_data = (char *)b;

        h->b_prev_free = h - 1;
        h->b_next_free = h + 1;
        h++;

        NR_BUFFERS++;

        if (b == (void *)0x100000)
            b = (void *)0xa0000;
    }

    h--; // h 时最后一个有效的 buffer_head
    free_list = start_buffer;
    free_list->b_prev_free = h;
    h->b_next_free = free_list; // 构造环形双向链表

    // reset hash table
    for (i = 0; i < NR_HASH; i++) {
        hash_table[i] = NULL;
    }
}
