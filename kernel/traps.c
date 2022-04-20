#include <asm/system.h>
#include <linux/kernel.h>

void page_fault(void); // int14(mm/page.s)

extern void *idt_ptrs[256];

/**
 * 异常(陷阱)中断程序初始化
 * 设置它们的中断调用门(中断向量)。set_trap_gate()与set_system_gate()都使用了中断描述符表IDT中
 * 的陷阱门(Trap
 * Gate)，它们之间的主要区别在于前者设置的特权级为0，后者是3。因此断点陷阱中断int3，
 * 溢出中断overflow和边界出错中断bounds可以由任何程序调用。
 */
void trap_init(void) {
  int i = 0;
  for (; i < 256; i++) {
    unsigned long off = (unsigned long)idt_ptrs[i];
    idt[i].a = (idt[i].a & (~0xffffL)) | (off & 0xffffL);
  }
  set_trap_gate(14, &page_fault);
}

int sys_setup() { return 0; }
int sys_exit() { return 0; }
int sys_read() { return 0; }
int sys_write() { return 0; }
int sys_open() { return 0; }
int sys_close() { return 0; }
int sys_waitpid() { return 0; }
int sys_creat() { return 0; }
int sys_link() { return 0; }
int sys_unlink() { return 0; }
int sys_execve() { return 0; }
int sys_chdir() { return 0; }
int sys_time() { return 0; }
int sys_mknod() { return 0; }
int sys_chmod() { return 0; }
int sys_chown() { return 0; }
int sys_break() { return 0; }
int sys_stat() { return 0; }
int sys_lseek() { return 0; }
int sys_getpid() { return 0; }
int sys_mount() { return 0; }
int sys_umount() { return 0; }
int sys_setuid() { return 0; }
int sys_getuid() { return 0; }
int sys_stime() { return 0; }
int sys_ptrace() { return 0; }
int sys_alarm() { return 0; }
int sys_fstat() { return 0; }
int sys_utime() { return 0; }
int sys_stty() { return 0; }
int sys_gtty() { return 0; }
int sys_access() { return 0; }
int sys_nice() { return 0; }
int sys_ftime() { return 0; }
int sys_sync() { return 0; }
int sys_kill() { return 0; }
int sys_rename() { return 0; }
int sys_mkdir() { return 0; }
int sys_rmdir() { return 0; }
int sys_dup() { return 0; }
int sys_pipe() { return 0; }
int sys_times() { return 0; }
int sys_prof() { return 0; }
int sys_brk() { return 0; }
int sys_setgid() { return 0; }
int sys_getgid() { return 0; }
int sys_signal() { return 0; }
int sys_geteuid() { return 0; }
int sys_getegid() { return 0; }
int sys_acct() { return 0; }
int sys_phys() { return 0; }
int sys_lock() { return 0; }
int sys_ioctl() { return 0; }
int sys_fcntl() { return 0; }
int sys_mpx() { return 0; }
int sys_setpgid() { return 0; }
int sys_ulimit() { return 0; }
int sys_uname() { return 0; }
int sys_umask() { return 0; }
int sys_chroot() { return 0; }
int sys_ustat() { return 0; }
int sys_dup2() { return 0; }
int sys_getppid() { return 0; }
int sys_getpgrp() { return 0; }
int sys_setsid() { return 0; }
int sys_sigaction() { return 0; }
int sys_sgetmask() { return 0; }
int sys_ssetmask() { return 0; }
int sys_setreuid() { return 0; }
int sys_setregid() { return 0; }
int sys_sigpending() { return 0; }
int sys_sigsuspend() { return 0; }
int sys_sethostname() { return 0; }
int sys_setrlimit() { return 0; }
int sys_getrlimit() { return 0; }
int sys_getrusage() { return 0; }
int sys_gettimeofday() { return 0; }
int sys_settimeofday() { return 0; }
int sys_getgroups() { return 0; }
int sys_setgroups() { return 0; }
int sys_select() { return 0; }
int sys_symlink() { return 0; }
int sys_lstat() { return 0; }
int sys_readlink() { return 0; }
int sys_uselib() { return 0; }
