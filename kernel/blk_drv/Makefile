include ../../rules.mk
include ../../wd.mk

CFLAGS	= -O -Wall -fstrength-reduce -fomit-frame-pointer \
	-finline-functions -nostdinc -fno-builtin -I../../include

CPP += -E -I../../include

blk_drv.a: $(objs) 
	$(AR) rcs $@ $(objs)

clean:
	@rm -f *.o
	@rm -f *.d
	@rm -f *.a

-include $(deps)

