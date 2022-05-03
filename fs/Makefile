include ../rules.mk
include ../wd.mk

CFLAGS	= -O -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I../include

CPP += -E -I../include

fs.o: $(objs) 
	$(LD) -r -o $@ $^

clean:
	@rm -f *.o
	@rm -f *.d

-include $(deps)

