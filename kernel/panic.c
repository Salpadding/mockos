#include <asm/system.h>
#include <linux/kernel.h>

void panic(const char *str) {
    printm(str);
    asmbp();
}
