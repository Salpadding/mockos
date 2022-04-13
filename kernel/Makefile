include ../rules.mk
include ../wd.mk

CFLAGS	= -O -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I../include

CPP += -E -I../include

kernel.o: $(objs) sys_call.o nasm.o idt.o
	$(LD) -r -o $@ $^

clean:
	@rm -f *.o
	@rm -f *.d

nasm.o: nasm.asm
	nasm -f elf32 -o $@ $^

idt.o: idt.asm
	nasm -f elf32 -o $@ $^

sys_call.o: sys_call.s

-include $(deps)

