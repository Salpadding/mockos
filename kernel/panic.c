#include <linux/kernel.h>

void panic(const char *str) {
 printm(str);
    
  // __asm__("cli\n\t"
  //         "hlt\n\t");
}
