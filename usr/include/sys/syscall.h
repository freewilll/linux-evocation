#ifndef	_SYS_SYSCALL_H
#define	_SYS_SYSCALL_H

#define	SYS_setup		0 /* Used only by init, to get system going. */
#define SYS_exit		1
#define SYS_fork		2
#define SYS_read		3
#define SYS_write		4
#define SYS_open		5
#define SYS_close		6
#define SYS_waitpid		7
#define SYS_creat		8
#define SYS_link		9
#define SYS_unlink		10
#define SYS_execve		11
#define SYS_chdir		12
#define SYS_time		13
#define SYS_mknod		14
#define SYS_chmod		15
#define SYS_chown		16
#define SYS_break		17
#define SYS_oldstat		18
#define SYS_lseek		19
#define SYS_getpid		20
#define SYS_mount		21
#define SYS_umount		22
#define SYS_setuid		23
#define SYS_getuid		24
#define SYS_stime		25
#define SYS_ptrace		26
#define SYS_alarm		27
#define SYS_oldfstat		28
#define SYS_pause		29
#define SYS_utime		30
#define SYS_stty		31
#define SYS_gtty		32
#define SYS_access		33
#define SYS_nice		34
#define SYS_ftime		35
#define SYS_sync		36
#define SYS_kill		37
#define SYS_rename		38
#define SYS_mkdir		39
#define SYS_rmdir		40
#define SYS_dup			41
#define SYS_pipe		42
#define SYS_times		43
#define SYS_prof		44
#define SYS_brk			45
#define SYS_setgid		46
#define SYS_getgid		47
#define SYS_signal		48
#define SYS_geteuid		49
#define SYS_getegid		50
#define SYS_acct		51
#define SYS_phys		52
#define SYS_lock		53
#define SYS_ioctl		54
#define SYS_fcntl		55
#define SYS_mpx			56
#define SYS_setpgid		57
#define SYS_ulimit		58
#define SYS_oldolduname		59
#define SYS_umask		60
#define SYS_chroot		61
#define SYS_ustat		62
#define SYS_dup2		63
#define SYS_getppid		64
#define SYS_getpgrp		65
#define SYS_setsid		66
#define SYS_sigaction		67
#define SYS_siggetmask		68
#define SYS_sigsetmask		69
#define SYS_setreuid		70
#define SYS_setregid		71
#define SYS_sigsuspend		72
#define SYS_sigpending		73
#define SYS_sethostname		74
#define SYS_setrlimit		75
#define SYS_getrlimit		76
#define SYS_getrusage		77
#define SYS_gettimeofday	78
#define SYS_settimeofday	79
#define SYS_getgroups		80
#define SYS_setgroups		81
#define SYS_select		82
#define SYS_symlink		83
#define SYS_oldlstat		84
#define SYS_readlink		85
#define SYS_uselib		86
#define SYS_swapon		87
#define SYS_reboot		88
#define SYS_readdir		89
#define SYS_mmap		90
#define SYS_munmap		91
#define SYS_truncate		92
#define SYS_ftruncate		93
#define SYS_fchmod		94
#define SYS_fchown		95
#define SYS_getpriority		96
#define SYS_setpriority		97
#define SYS_profil		98
#define SYS_statfs		99
#define SYS_fstatfs		100
#define SYS_ioperm		101
#define SYS_socketcall		102
#define SYS_klog		103
#define SYS_setitimer		104
#define SYS_getitimer		105
#define SYS_stat		106
#define SYS_lstat		107
#define SYS_fstat		108
#define SYS_olduname		109
#define SYS_iopl		110
#define SYS_vhangup		111
#define SYS_idle		112
#define SYS_vm86		113
#define SYS_wait4		114
#define SYS_swapoff		115
#define SYS_sysinfo		116
#define SYS_ipc			117
#define SYS_fsync		118
#define SYS_sigreturn		119
#define SYS_clone		120
#define SYS_setdomainname	121
#define SYS_uname		122


#endif	/* <sys/syscall.h> */
