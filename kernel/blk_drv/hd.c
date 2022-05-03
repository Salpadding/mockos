#include <asm/io.h>
#include <asm/system.h>
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <linux/kernel.h>

// 利用宏减少重复的代码
#define MAJOR_NR 3
#include "blk.h"

#define MAX_ERRORS 7
#define MAX_HD 2

static int recalibrate = 0;
static int reset = 0;

char hd_test_buf[1024];

static struct buffer_head test_bh = {hd_test_buf, 0,    0x300,    0,    0,    1,
                                     0,        NULL, NULL, NULL, NULL, NULL};

#define port_read(port, buf, nr)                                               \
    __asm__("cld;rep;insw" ::"d"(port), "D"(buf), "c"(nr))

#define port_write(port, buf, nr)                                              \
    __asm__("cld;rep;outsw" ::"d"(port), "S"(buf), "c"(nr))

static int win_result(void) {
    int i = inb_p(HD_STATUS);

    if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT)) ==
        (READY_STAT | SEEK_STAT))
        return (0); /* ok */
    if (i & 1)
        i = inb(HD_ERROR);
    return (1);
}

static void read_intr(void) {
    if (win_result()) {
        printm("bad rw intr\n");
        return;
    }

    port_read(HD_DATA, CURRENT->buffer, 256); // 一次读 256 个 word = 512 bytes
    printm("read into buffer success\n");
    CURRENT->errors = 0;
    CURRENT->buffer += 512;
    CURRENT->sector++;

    if (--(CURRENT->nr_sectors)) {
        SET_INTR(&read_intr);
        return;
    }
}

extern void hd_interrupt();

int hd_timeout;

void unexpected_hd_int() {
    printm("unexpected int");
    __asm__("cli\n\thlt\n\t");
}

void hd_init(void) {
    set_intr_gate(0x2E, &hd_interrupt);
    outb_p(inb_p(0x21) & 0xfb, 0x21);
    outb(inb_p(0xA1) & 0xbf, 0xA1);
}


static int mock_hd_out() {
    SET_INTR(&read_intr); // 修改 do_hd
    outb_p(0xc8, 0x3f6); // 每次都 reset 一下 control block
    outb_p(0xff, 0x1f1); // 不清楚 应该和异常处理有关
    outb_p(0x02, 0x1f2); // 写入要读的扇区数量
    // lba address
    outb_p(0x01, 0x1f3);
    outb_p(0, 0x1f4);
    outb_p(0, 0x1f5);
    outb_p(0xa0, 0x1f6);

    // 发出读的指令
    outb(0x20, 0x1f7);

    return 0;
}

static int sys_kdebug() {
    // mock 一个没有被占用的 buffer_head
    struct buffer_head* bh = &test_bh;

    // mock 一个 read request, 模仿 make_request 的行为
    struct request* req = &request[31];
    req->dev = bh->b_dev;
    req->cmd = READ;
    req->errors = 0;
    req->sector = bh->b_blocknr << 1;
    req->nr_sectors = 2;
    req->buffer = bh->b_data;
    req->bh = bh;
    req->next = NULL;

    // 模仿 add_request 和 do_hd_request 的 行为 
    struct blk_dev_struct* dev = &blk_dev[3];
    // 把当前 request 指向 req
    dev->current_request = req;

    // 模仿 hd_out
    mock_hd_out(); 

    return 0;
}

