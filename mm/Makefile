include ../rules.mk
include ../wd.mk

CFLAGS	= -g -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I../include

CPP += -E -I../include

mm.o: $(objs)
	$(LD) -r -o $@ $^

clean:
	@rm -f *.o

-include $(deps)
