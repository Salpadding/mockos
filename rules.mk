AS	= as --32            # --32: 64位系统需要加
LD	= ld -m elf_i386     # -m elf_i386: 64位系统需要加
AR	= ar
CC	= gcc-3.4 -m32           # -m32: 64位系统需要加
CPP	= cpp -nostdinc      # -nostdinc: 不要去搜索标准头文件目录，即不使用标准头文件
