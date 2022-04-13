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
#include <asm/system.h>
#include <linux/kernel.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

static char buf[1024]; /* 显示用的临时缓冲区 */

/* 内核使用的显示函数 */
int printk(const char *fmt, ...) {
  va_list args;
  int i;

  va_start(args, fmt);
  i = vsprintf(buf, fmt, args);
  va_end(args);
  // console_print(buf);
  return i;
}

/* 内核使用的显示函数 */
int printm(const char *fmt, ...) {
  int i, j;
  char buf[256];
  va_list args;
  va_start(args, fmt);
  i = vsprintf(buf, fmt, args);
  va_end(args);

  log_buf(buf, i);
  return i;
}

/* 内核使用的显示函数 */
