/*
 * Why isn't this a .c file?  Enquiring minds....
 */

#define sys_clone sys_fork

// TODO WGJA WIP: extern "C" {
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int sys_setup();
// TODO WGJA WIP: extern int sys_exit();
// TODO WGJA WIP: extern int sys_fork();
// TODO WGJA WIP: extern int sys_read();
// TODO WGJA WIP: extern int sys_write();
// TODO WGJA WIP: extern int sys_open();
// TODO WGJA WIP: extern int sys_close();
// TODO WGJA WIP: extern int sys_waitpid();
// TODO WGJA WIP: extern int sys_creat();
// TODO WGJA WIP: extern int sys_link();
// TODO WGJA WIP: extern int sys_unlink();
// TODO WGJA WIP: extern int sys_execve();
// TODO WGJA WIP: extern int sys_chdir();
// TODO WGJA WIP: extern int sys_time();
// TODO WGJA WIP: extern int sys_mknod();
// TODO WGJA WIP: extern int sys_chmod();
// TODO WGJA WIP: extern int sys_chown();
// TODO WGJA WIP: extern int sys_break();
// TODO WGJA WIP: extern int sys_stat();
// TODO WGJA WIP: extern int sys_lseek();
// TODO WGJA WIP: extern int sys_getpid();
// TODO WGJA WIP: extern int sys_mount();
// TODO WGJA WIP: extern int sys_umount();
// TODO WGJA WIP: extern int sys_setuid();
// TODO WGJA WIP: extern int sys_getuid();
// TODO WGJA WIP: extern int sys_stime();
// TODO WGJA WIP: extern int sys_ptrace();
// TODO WGJA WIP: extern int sys_alarm();
// TODO WGJA WIP: extern int sys_fstat();
// TODO WGJA WIP: extern int sys_pause();
// TODO WGJA WIP: extern int sys_utime();
// TODO WGJA WIP: extern int sys_stty();
// TODO WGJA WIP: extern int sys_gtty();
// TODO WGJA WIP: extern int sys_access();
// TODO WGJA WIP: extern int sys_nice();
// TODO WGJA WIP: extern int sys_ftime();
// TODO WGJA WIP: extern int sys_sync();
// TODO WGJA WIP: extern int sys_kill();
// TODO WGJA WIP: extern int sys_rename();
// TODO WGJA WIP: extern int sys_mkdir();
// TODO WGJA WIP: extern int sys_rmdir();
// TODO WGJA WIP: extern int sys_dup();
// TODO WGJA WIP: extern int sys_pipe();
// TODO WGJA WIP: extern int sys_times();
// TODO WGJA WIP: extern int sys_prof();
// TODO WGJA WIP: extern int sys_brk();
// TODO WGJA WIP: extern int sys_setgid();
// TODO WGJA WIP: extern int sys_getgid();
// TODO WGJA WIP: extern int sys_signal();
// TODO WGJA WIP: extern int sys_geteuid();
// TODO WGJA WIP: extern int sys_getegid();
// TODO WGJA WIP: extern int sys_acct();
// TODO WGJA WIP: extern int sys_phys();
// TODO WGJA WIP: extern int sys_lock();
// TODO WGJA WIP: extern int sys_ioctl();
// TODO WGJA WIP: extern int sys_fcntl();
// TODO WGJA WIP: extern int sys_mpx();
// TODO WGJA WIP: extern int sys_setpgid();
// TODO WGJA WIP: extern int sys_ulimit();
// TODO WGJA WIP: extern int sys_uname();
// TODO WGJA WIP: extern int sys_umask();
// TODO WGJA WIP: extern int sys_chroot();
// TODO WGJA WIP: extern int sys_ustat();
// TODO WGJA WIP: extern int sys_dup2();
// TODO WGJA WIP: extern int sys_getppid();
// TODO WGJA WIP: extern int sys_getpgrp();
// TODO WGJA WIP: extern int sys_setsid();
// TODO WGJA WIP: extern int sys_sigaction();
// TODO WGJA WIP: extern int sys_sgetmask();
// TODO WGJA WIP: extern int sys_ssetmask();
// TODO WGJA WIP: extern int sys_setreuid();
// TODO WGJA WIP: extern int sys_setregid();
// TODO WGJA WIP: extern int sys_sigpending();
// TODO WGJA WIP: extern int sys_sigsuspend();
// TODO WGJA WIP: extern int sys_sethostname();
// TODO WGJA WIP: extern int sys_setrlimit();
// TODO WGJA WIP: extern int sys_getrlimit();
// TODO WGJA WIP: extern int sys_getrusage();
// TODO WGJA WIP: extern int sys_gettimeofday();
// TODO WGJA WIP: extern int sys_settimeofday();
// TODO WGJA WIP: extern int sys_getgroups();
// TODO WGJA WIP: extern int sys_setgroups();
// TODO WGJA WIP: extern int sys_select();
// TODO WGJA WIP: extern int sys_symlink();
// TODO WGJA WIP: extern int sys_lstat();
// TODO WGJA WIP: extern int sys_readlink();
// TODO WGJA WIP: extern int sys_uselib();
// TODO WGJA WIP: extern int sys_swapon();
// TODO WGJA WIP: extern int sys_reboot();
// TODO WGJA WIP: extern int sys_readdir();
// TODO WGJA WIP: extern int sys_mmap();
// TODO WGJA WIP: extern int sys_munmap();
// TODO WGJA WIP: extern int sys_truncate();
// TODO WGJA WIP: extern int sys_ftruncate();
// TODO WGJA WIP: extern int sys_fchmod();
// TODO WGJA WIP: extern int sys_fchown();
// TODO WGJA WIP: extern int sys_getpriority();
// TODO WGJA WIP: extern int sys_setpriority();
// TODO WGJA WIP: extern int sys_profil();
// TODO WGJA WIP: extern int sys_statfs();
// TODO WGJA WIP: extern int sys_fstatfs();
// TODO WGJA WIP: extern int sys_ioperm();
// TODO WGJA WIP: extern int sys_socketcall();
// TODO WGJA WIP: extern int sys_syslog();
// TODO WGJA WIP: extern int sys_getitimer();
// TODO WGJA WIP: extern int sys_setitimer();
// TODO WGJA WIP: extern int sys_newstat();
// TODO WGJA WIP: extern int sys_newlstat();
// TODO WGJA WIP: extern int sys_newfstat();
// TODO WGJA WIP: extern int sys_newuname();
// TODO WGJA WIP: extern int sys_iopl();
// TODO WGJA WIP: extern int sys_vhangup();
// TODO WGJA WIP: extern int sys_idle();
// TODO WGJA WIP: extern int sys_vm86();
// TODO WGJA WIP: extern int sys_wait4();
// TODO WGJA WIP: extern int sys_swapoff();
// TODO WGJA WIP: extern int sys_sysinfo();
// TODO WGJA WIP: extern int sys_ipc();
// TODO WGJA WIP: extern int sys_fsync();
// TODO WGJA WIP: extern int sys_sigreturn();
// TODO WGJA WIP: extern int sys_setdomainname();
// TODO WGJA WIP: extern int sys_olduname();
// TODO WGJA WIP: extern int sys_old_syscall();
// TODO WGJA WIP: extern int sys_modify_ldt();
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * These are system calls that will be removed at some time
// TODO WGJA WIP:  * due to newer versions existing..
// TODO WGJA WIP:  */
// TODO WGJA WIP: #ifdef notdef
// TODO WGJA WIP: #define sys_waitpid	sys_old_syscall	/* sys_wait4	*/
// TODO WGJA WIP: #define sys_olduname	sys_old_syscall /* sys_newuname	*/
// TODO WGJA WIP: #define sys_stat	sys_old_syscall /* sys_newstat	*/
// TODO WGJA WIP: #define sys_fstat	sys_old_syscall	/* sys_newfstat	*/
// TODO WGJA WIP: #define sys_lstat	sys_old_syscall /* sys_newlstat	*/
// TODO WGJA WIP: #define sys_signal	sys_old_syscall	/* sys_sigaction */
// TODO WGJA WIP: #endif

typedef int (*fn_ptr)();


// TODO WGJA syscalls
fn_ptr sys_call_table[] = {};

// TODO WGJA WIP: fn_ptr sys_call_table[] = { sys_setup, sys_exit, sys_fork, sys_read,
// TODO WGJA WIP: sys_write, sys_open, sys_close, sys_waitpid, sys_creat, sys_link,
// TODO WGJA WIP: sys_unlink, sys_execve, sys_chdir, sys_time, sys_mknod, sys_chmod,
// TODO WGJA WIP: sys_chown, sys_break, sys_stat, sys_lseek, sys_getpid, sys_mount,
// TODO WGJA WIP: sys_umount, sys_setuid, sys_getuid, sys_stime, sys_ptrace, sys_alarm,
// TODO WGJA WIP: sys_fstat, sys_pause, sys_utime, sys_stty, sys_gtty, sys_access,
// TODO WGJA WIP: sys_nice, sys_ftime, sys_sync, sys_kill, sys_rename, sys_mkdir,
// TODO WGJA WIP: sys_rmdir, sys_dup, sys_pipe, sys_times, sys_prof, sys_brk, sys_setgid,
// TODO WGJA WIP: sys_getgid, sys_signal, sys_geteuid, sys_getegid, sys_acct, sys_phys,
// TODO WGJA WIP: sys_lock, sys_ioctl, sys_fcntl, sys_mpx, sys_setpgid, sys_ulimit,
// TODO WGJA WIP: sys_olduname, sys_umask, sys_chroot, sys_ustat, sys_dup2, sys_getppid,
// TODO WGJA WIP: sys_getpgrp, sys_setsid, sys_sigaction, sys_sgetmask, sys_ssetmask,
// TODO WGJA WIP: sys_setreuid,sys_setregid, sys_sigsuspend, sys_sigpending,
// TODO WGJA WIP: sys_sethostname, sys_setrlimit, sys_getrlimit, sys_getrusage,
// TODO WGJA WIP: sys_gettimeofday, sys_settimeofday, sys_getgroups, sys_setgroups,
// TODO WGJA WIP: sys_select, sys_symlink, sys_lstat, sys_readlink, sys_uselib,
// TODO WGJA WIP: sys_swapon, sys_reboot, sys_readdir, sys_mmap, sys_munmap, sys_truncate,
// TODO WGJA WIP: sys_ftruncate, sys_fchmod, sys_fchown, sys_getpriority, sys_setpriority,
// TODO WGJA WIP: sys_profil, sys_statfs, sys_fstatfs, sys_ioperm, sys_socketcall,
// TODO WGJA WIP: sys_syslog, sys_setitimer, sys_getitimer, sys_newstat, sys_newlstat,
// TODO WGJA WIP: sys_newfstat, sys_uname, sys_iopl, sys_vhangup, sys_idle, sys_vm86,
// TODO WGJA WIP: sys_wait4, sys_swapoff, sys_sysinfo, sys_ipc, sys_fsync, sys_sigreturn,
// TODO WGJA WIP: sys_clone, sys_setdomainname, sys_newuname, sys_modify_ldt};
// TODO WGJA WIP: 
// TODO WGJA WIP: }

/* So we don't have to do any more manual updating.... */
int NR_syscalls = sizeof(sys_call_table)/sizeof(fn_ptr);
