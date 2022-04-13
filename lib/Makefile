include ../rules.mk
include ../wd.mk

CFLAGS	= -O -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I../include

CPP += -E -I../include

lib.a: $(objs) 
	$(AR) rcs lib.a $^

clean:
	@rm -f *.o
	@rm -f *.a
	@rm -f *.d


-include $(deps)

