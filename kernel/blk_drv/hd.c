#include <asm/system.h>
#include <asm/io.h>
#include <linux/kernel.h>

extern void hd_interrupt();

int hd_timeout;

void (*do_hd)(void) = (void *)0;


void unexpected_hd_int() {

}


void __hd_int() {
    printm("__hd_int\n");
}


void hd_init(void) {
    set_intr_gate(0x2E, &hd_interrupt);
    outb_p(inb_p(0x21) & 0xfb, 0x21);
    outb(inb_p(0xA1) & 0xbf, 0xA1);
}
