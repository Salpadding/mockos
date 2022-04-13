include ../rules.mk
include ../wd.mk

CFLAGS	= -g -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I../include

CPP += -E -I../include

mm.o: $(objs) page.o
	$(LD) -r -o $@ $^

page.o: page.asm
	nasm -f elf32 -o $@ $^

clean:
	@rm -f *.o
	@rm -f *.d

-include $(deps)
