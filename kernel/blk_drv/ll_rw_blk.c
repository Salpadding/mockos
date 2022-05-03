#include "blk.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

/*
 * The request-struct contains all necessary data
 * to load a nr of sectors into memory
 */
/*
 * 请求结构中含有加载 nr 个扇区数据到内存中去的所有必须的信息。
 */
/* 请求项数组队列，共有NR_REQUEST = 32个请求项 */
struct request request[NR_REQUEST];

/* blk_dev_struct is:
 *	do_request-address
 *	next-request
 */
/*
 * blk_dev_struct块设备结构是:(参见文件kernel/blk_drv/blk.h)
 * do_request-address	// 对应主设备号的请求处理程序指针
 * current-request		// 该设备的下一个请求
 */
// 块设备数组。该数组使用主设备号作为索引。实际内容将在各块设备驱动程序初始化时填入。
// 例如，硬盘驱动程序初始化时(hd.c)，第一条语句即用于设备blk_dev[3]的内容。
struct blk_dev_struct blk_dev[NR_BLK_DEV] = {
    {NULL, NULL},
    /* no_dev */ /* 0 - 无设备 */
    {NULL, NULL},
    /* dev mem */ /* 1 - 内存 */
    {NULL, NULL},
    /* dev fd */ /* 2 - 软驱设备 */
    {NULL, NULL},
    /* dev hd */ /* 3 - 硬盘设备 */
    {NULL, NULL},
    /* dev ttyx */ /* 4 - ttyx设备 */
    {NULL, NULL},
    /* dev tty */             /* 5 - tty设备 */
    {NULL, NULL} /* dev lp */ /* 6 - lp打印机设备 */
};
