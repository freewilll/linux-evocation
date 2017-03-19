/*
 * Why isn't this a .c file?  Enquiring minds....
 */

#define sys_clone sys_fork

extern "C" {

extern int sys_todo();
extern int sys_setup();
extern int sys_exit();
extern int sys_fork();
extern int sys_read();
extern int sys_write();
extern int sys_open(const char *, int, int);

extern int sys_close(unsigned int);
extern int sys_waitpid();
extern int sys_creat();
extern int sys_link();
extern int sys_unlink();
extern int sys_execve();
extern int sys_chdir();
extern int sys_time();
extern int sys_mknod();
extern int sys_chmod();
extern int sys_chown();
extern int sys_break();
extern int sys_stat();
extern int sys_lseek();
extern int sys_getpid();
extern int sys_mount();
extern int sys_umount();
extern int sys_setuid();
extern int sys_getuid();
extern int sys_stime();
extern int sys_ptrace();
extern int sys_alarm();
extern int sys_fstat();
extern int sys_pause();
extern int sys_utime();
extern int sys_stty();
extern int sys_gtty();
extern int sys_access();
extern int sys_nice(long int);
extern int sys_ftime();
extern int sys_sync();
extern int sys_kill();
extern int sys_rename();
extern int sys_mkdir();
extern int sys_rmdir();
extern int sys_dup();
extern int sys_pipe();
extern int sys_times();
extern int sys_prof();
extern int sys_brk();
extern int sys_setgid();
extern int sys_getgid();
extern int sys_signal();
extern int sys_geteuid();
extern int sys_getegid();
extern int sys_acct();
extern int sys_phys();
extern int sys_lock();
extern int sys_ioctl();
extern int sys_fcntl();
extern int sys_mpx();
extern int sys_setpgid();
extern int sys_ulimit();
extern int sys_uname();
extern int sys_umask();
extern int sys_chroot();
extern int sys_ustat();
extern int sys_dup2();
extern int sys_getppid();
extern int sys_getpgrp();
extern int sys_setsid();
extern int sys_sigaction();
extern int sys_sgetmask();
extern int sys_ssetmask();
extern int sys_setreuid();
extern int sys_setregid();
extern int sys_sigpending();
extern int sys_sigsuspend();
extern int sys_sethostname();
extern int sys_setrlimit();
extern int sys_getrlimit();
extern int sys_getrusage();
extern int sys_gettimeofday();
extern int sys_settimeofday();
extern int sys_getgroups();
extern int sys_setgroups();
extern int sys_select();
extern int sys_symlink();
extern int sys_lstat();
extern int sys_readlink();
extern int sys_uselib();
extern int sys_swapon();
extern int sys_reboot();
extern int sys_readdir();
extern int sys_mmap();
extern int sys_munmap();
extern int sys_truncate();
extern int sys_ftruncate();
extern int sys_fchmod();
extern int sys_fchown();
extern int sys_getpriority();
extern int sys_setpriority();
extern int sys_profil();
extern int sys_statfs();
extern int sys_fstatfs();
extern int sys_ioperm();
extern int sys_socketcall();
extern int sys_syslog();
extern int sys_getitimer();
extern int sys_setitimer();
extern int sys_newstat();
extern int sys_newlstat();
extern int sys_newfstat();
extern int sys_newuname();
extern int sys_iopl();
extern int sys_vhangup();
extern int sys_idle();
extern int sys_vm86();
extern int sys_wait4();
extern int sys_swapoff();
extern int sys_sysinfo();
extern int sys_ipc();
extern int sys_fsync();
extern int sys_sigreturn();
extern int sys_setdomainname();
extern int sys_olduname();
extern int sys_old_syscall();
extern int sys_modify_ldt();

/*
 * These are system calls that will be removed at some time
 * due to newer versions existing..
 */
#ifdef notdef
#define sys_waitpid	sys_old_syscall	/* sys_wait4	*/
#define sys_olduname	sys_old_syscall /* sys_newuname	*/
#define sys_stat	sys_old_syscall /* sys_newstat	*/
#define sys_fstat	sys_old_syscall	/* sys_newfstat	*/
#define sys_lstat	sys_old_syscall /* sys_newlstat	*/
#define sys_signal	sys_old_syscall	/* sys_sigaction */
#endif

typedef int (*fn_ptr)();

fn_ptr sys_call_table[] = {
	sys_setup,		// __NR_setup		0
	sys_exit,		// __NR_exit		1
	sys_fork,		// __NR_fork		2
	(fn_ptr) sys_read,	// __NR_read		3
	(fn_ptr) sys_write,	// __NR_write		4
	(fn_ptr) sys_open,	// __NR_open		5
	(fn_ptr) sys_close,	// __NR_close		6
	sys_waitpid,		// __NR_waitpid		7
	sys_creat,		// __NR_creat		8
	sys_link,		// __NR_link		9
	sys_unlink,		// __NR_unlink		10
	sys_execve,		// __NR_execve		11
	sys_chdir,		// __NR_chdir		12
	sys_time,		// __NR_time		13
	sys_mknod,		// __NR_mknod		14
	sys_chmod,		// __NR_chmod		15
	sys_chown,		// __NR_chown		16
	sys_break,		// __NR_break		17
	sys_stat,		// __NR_oldstat		18
	sys_lseek,		// __NR_lseek		19
	sys_getpid,		// __NR_getpid		20
	sys_mount,		// __NR_mount		21
	sys_umount,		// __NR_umount		22
	sys_setuid,		// __NR_setuid		23
	sys_getuid,		// __NR_getuid		24
	sys_stime,		// __NR_stime		25
	sys_ptrace,		// __NR_ptrace		26	todo
	sys_alarm,		// __NR_alarm		27	todo
	sys_fstat,		// __NR_oldfstat	28
	sys_pause,		// __NR_pause		29
	sys_utime,		// __NR_utime		30
	sys_stty,		// __NR_stty		31
	sys_gtty,		// __NR_gtty		32
	sys_access,		// __NR_access		33
	(fn_ptr) sys_nice,	// __NR_nice		34
	sys_ftime,		// __NR_ftime		35
	sys_sync,		// __NR_sync		36
	sys_kill,		// __NR_kill		37
	sys_rename,		// __NR_rename		38
	sys_mkdir,		// __NR_mkdir		39
	sys_rmdir,		// __NR_rmdir		40
	sys_dup,		// __NR_dup		41
	sys_pipe,		// __NR_pipe		42
	sys_times,		// __NR_times		43
	sys_prof,		// __NR_prof		44
	sys_brk,		// __NR_brk		45
	sys_setgid,		// __NR_setgid		46
	sys_getgid,		// __NR_getgid		47
	sys_signal,		// __NR_signal		48
	sys_geteuid,		// __NR_geteuid		49
	sys_getegid,		// __NR_getegid		50
	sys_acct,		// __NR_acct		51
	sys_phys,		// __NR_phys		52
	sys_lock,		// __NR_lock		53
	sys_ioctl,		// __NR_ioctl		54
	sys_fcntl,		// __NR_fcntl		55	todo
	sys_mpx,		// __NR_mpx		56	todo
	sys_setpgid,		// __NR_setpgid		57
	sys_ulimit,		// __NR_ulimit		58
	sys_olduname,		// __NR_oldolduname	59
	sys_umask,		// __NR_umask		60
	sys_chroot,		// __NR_chroot		61
	sys_ustat,		// __NR_ustat		62
	sys_dup2,		// __NR_dup2		63
	sys_getppid,		// __NR_getppid		64
	sys_getpgrp,		// __NR_getpgrp		65
	sys_setsid,		// __NR_setsid		66
	sys_sigaction,		// __NR_sigaction	67
	sys_sgetmask,		// __NR_sgetmask	68
	sys_ssetmask,		// __NR_ssetmask	69
	sys_setreuid,		// __NR_setreuid	70
	sys_setregid,		// __NR_setregid	71
	sys_sigsuspend,		// __NR_sigsuspend	72
	sys_sigpending,		// __NR_sigpending	73
	sys_sethostname,	// __NR_sethostname	74
	sys_setrlimit,		// __NR_setrlimit	75
	sys_getrlimit,		// __NR_getrlimit	76
	sys_getrusage,		// __NR_getrusage	77
	sys_gettimeofday,	// __NR_gettimeofday	78
	sys_settimeofday,	// __NR_settimeofday	79
	sys_getgroups,		// __NR_getgroups	80
	sys_setgroups,		// __NR_setgroups	81
	sys_select,		// __NR_select		82	todo
	sys_symlink,		// __NR_symlink		83
	sys_lstat,		// __NR_oldlstat	84
	sys_readlink,		// __NR_readlink	85
	sys_uselib,		// __NR_uselib		86	todo
	sys_swapon,		// __NR_swapon		87	todo
	sys_reboot,		// __NR_reboot		88
	sys_readdir,		// __NR_readdir		89
	sys_mmap,		// __NR_mmap		90
	sys_munmap,		// __NR_munmap		91
	sys_truncate,		// __NR_truncate	92
	sys_ftruncate,		// __NR_ftruncate	93
	sys_fchmod,		// __NR_fchmod		94
	sys_fchown,		// __NR_fchown		95
	sys_getpriority,	// __NR_getpriority	96
	sys_setpriority,	// __NR_setpriority	97
	sys_profil,		// __NR_profil		98
	sys_statfs,		// __NR_statfs		99
	sys_fstatfs,		// __NR_fstatfs		100
	sys_ioperm,		// __NR_ioperm		101
	sys_socketcall,		// __NR_socketcall	102	todo
	sys_syslog,		// __NR_syslog		103	todo
	sys_setitimer,		// __NR_setitimer	104	todo
	sys_getitimer,		// __NR_getitimer	105	todo
	sys_newstat,		// __NR_stat		106
	sys_newlstat,		// __NR_lstat		107
	sys_newfstat,		// __NR_fstat		108
	sys_uname,		// __NR_olduname	109
	sys_iopl,		// __NR_iopl		110
	sys_vhangup,		// __NR_vhangup		111
	sys_idle,		// __NR_idle		112
	sys_vm86,		// __NR_vm86		113
	sys_wait4,		// __NR_wait4		114
	sys_swapoff,		// __NR_swapoff		115	todo
	sys_sysinfo,		// __NR_sysinfo		116	todo
	sys_ipc,		// __NR_ipc		117	todo
	sys_fsync,		// __NR_fsync		118
	sys_sigreturn,		// __NR_sigreturn	119
	sys_clone,		// __NR_clone		120
	sys_setdomainname,	// __NR_setdomainname	121
	sys_newuname,		// __NR_uname		122
	sys_modify_ldt,		// __NR_modify_ldt	123	todo
};

}

/* So we don't have to do any more manual updating.... */
int NR_syscalls = sizeof(sys_call_table)/sizeof(fn_ptr);
