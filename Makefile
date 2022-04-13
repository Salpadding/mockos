include rules.mk
LDFLAGS	= -M -x -Ttext 0 -e startup_32

# -g: 生成调试信息
# -Wall: 打印警告
# -O: 对代码进行优化
# -fstrength-reduce: 优化循环语句
# -fomit-frame-pointer: 对无需帧指针的函数不要把帧指针保留在寄存器中
# -fcombine-regs(去除): 不再被gcc支持
# -mstring-insns(去除): Linus本人增加的选项(gcc中没有)
# -fno-builtin(新增): 阻止gcc会把没有参数的printf优化成puts
CFLAGS = -Wall -O -fstrength-reduce -fomit-frame-pointer -fno-builtin -Iinclude

Image: boot/bootsect boot/setup tools/system tools/build
	@cp -f tools/system system.tmp > /dev/null
	@strip system.tmp > /dev/null # remove symbols
	@objcopy -O binary -R .note -R .comment system.tmp tools/kernel > /dev/null # drop elf headers
	@tools/build boot/bootsect boot/setup tools/kernel > Image 2>/dev/null
	@rm system.tmp
	@rm tools/kernel -f
	@sync > /dev/null

boot/head.o: boot/head.s 

lib/lib.a: FORCE
	@(cd lib; make)

kernel/kernel.o: FORCE
	@(cd kernel; make)

mm/mm.o: FORCE
	@(cd mm; make)

init/main.o: init/main.c
	($(CPP) -Iinclude -MM $^) > init/main.d
	$(CC) $(CFLAGS) -c -o $@ $^

tools/build: tools/build.c 
	$(CC) -o $@ $^

tools/system: boot/head.o init/main.o mm/mm.o kernel/kernel.o lib/lib.a
	$(LD) $(LDFLAGS) -o $@ $^ > /dev/null

.PHONY: clean FORCE

clean:
	@rm -f Image
	@rm -f init/main.o
	@rm -f tools/system
	@rm -f boot/head.o
	@rm -f tools/build
	@(cd mm; $(MAKE) clean)
	@(cd kernel; $(MAKE) clean)

-include init/main.d
