/*
 *  linux/kernel/printk.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * When in kernel-mode, we cannot use printf, as fs is liable to
 * point to 'interesting' things. Make a printf with fs-saving, and
 * all is well.
 */
#include <stdarg.h>
#include <stddef.h>

#include <linux/kernel.h>

static char buf[1024];	/* 显示用的临时缓冲区 */

/* 内核使用的显示函数 */
int printk(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	// console_print(buf);
	return i;
}

/* 内核使用的显示函数 */
int printm(const char *fmt, ...)
{
    int* cur = (void*) (15 * 1024 * 1024);
    int i;
    char* j = (void*) (15 * 1024 * 1024 + 16 + *cur);
	va_list args;
	va_start(args, fmt);
	i = vsprintf(j, fmt, args);
	va_end(args);
    *cur += i;
	return i;
}


