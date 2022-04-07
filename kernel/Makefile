include ../rules.mk
include ../wd.mk

CFLAGS	= -O -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I../include

CPP += -E -I../include

kernel.o: $(objs) sys_call.o
	$(LD) -r -o $@ $^

clean:
	@rm -f *.o

sys_call.o: sys_call.s

-include $(deps)

