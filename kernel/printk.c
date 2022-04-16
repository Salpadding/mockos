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
#include <asm/io.h>
#include <asm/system.h>
#include <linux/kernel.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

extern int get_cs();
extern void call_sys_log(void *, int);

#define COM1_PORT 0x3f8 // COM1

int init_serial() {
  outb(0x00, COM1_PORT + 1); // Disable all interrupts
  outb(0x80, COM1_PORT + 3); // Enable DLAB (set baud rate divisor)
  outb(0x03, COM1_PORT + 0); // Set divisor to 3 (lo byte) 38400 baud
  outb(0x00, COM1_PORT + 1); //                  (hi byte)
  outb(0x03, COM1_PORT + 3); // 8 bits, no parity, one stop bit
  outb(0xC7, COM1_PORT + 2); // Enable FIFO, clear them, with 14-byte threshold
  outb(0x0B, COM1_PORT + 4); // IRQs enabled, RTS/DSR set
  outb(0x1E, COM1_PORT + 4); // Set in loopback mode, test the serial chip
  outb(0xAE, COM1_PORT + 0); // Test serial chip (send byte 0xAE and check if
                             // serial returns same byte)

  // Check if serial is faulty (i.e: not same byte as sent)
  if (inb(COM1_PORT + 0) != 0xAE) {
    return 1;
  }

  // If serial is not faulty set it in normal operation mode
  // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
  outb(0x0F, COM1_PORT + 4);
  return 0;
}

int is_transmit_empty() { return inb(COM1_PORT + 5) & 0x20; }

void write_serial(char a) {
  while (is_transmit_empty() == 0)
    ;

  outb(a, COM1_PORT);
}

void sys_log_c(const char *buf, int len) {
  int j;
  for (j = 0; j < len; j++) {
    write_serial(buf[j]);
  }
}

/* 内核使用的显示函数 */
int printk(const char *fmt, ...) {
  va_list args;
  int i;
  int j;

  char buf[256];
  va_start(args, fmt);
  i = vsprintf(buf, fmt, args);
  va_end(args);
  // get cpl
  int cpl = get_cs() & 3;
  if (cpl != 0) {
    // user space
    call_sys_log(buf, i);
    return i;
  }

  for (j = 0; j < i; j++) {
    write_serial(buf[j]);
  }
  return i;
}

/* 内核使用的显示函数 */
int printm(const char *fmt, ...) {
  int i;
  char buf[256];
  va_list args;
  va_start(args, fmt);
  i = vsprintf(buf, fmt, args);
  va_end(args);

  log_buf(buf, i);
  return i;
}
