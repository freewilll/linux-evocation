#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <features.h>
#include <sys/types.h>
#undef __NOT_KERNEL
#define __NOT_KERNEL
#include <linux/stat.h>
#undef __NOT_KERNEL

__BEGIN_DECLS

#ifdef OLD_LINUX
struct stat {
	dev_t   st_dev;
	ino_t   st_ino;
	umode_t st_mode;
	nlink_t st_nlink;
	uid_t   st_uid;
	gid_t   st_gid;
	dev_t   st_rdev;
	off_t   st_size;
	time_t  st_atime;
	time_t  st_mtime;
	time_t  st_ctime;
};
#else
struct stat {
	dev_t		st_dev;
	unsigned short	__pad1;
	ino_t		st_ino;
	umode_t		st_mode;
	nlink_t		st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	dev_t		st_rdev;
	unsigned short	__pad2;
	off_t		st_size;
	unsigned long	st_blksize;
	unsigned long	st_blocks;
	time_t		st_atime;
	unsigned long	__unused1;
	time_t		st_mtime;
	unsigned long	__unused2;
	time_t		st_ctime;
	unsigned long	__unused3;
	unsigned long	__unused4;
	unsigned long	__unused5;
};
#endif

/* Some synonyms used historically in the kernel and elsewhere */
#define S_IREAD		S_IRUSR /* read permission, owner */
#define S_IWRITE	S_IWUSR /* write permission, owner */
#define S_IEXEC		S_IXUSR /* execute/search permission, owner */

extern int	__chmod __P ((__const char *__path, mode_t __mode));
extern int	chmod __P ((__const char *__path, mode_t __mode));

extern int	__fchmod __P ((int __fildes, mode_t __mode));
extern int	fchmod __P ((int __fildes, mode_t __mode));

extern int	__fstat __P ((int __fildes, struct stat *__stat_buf));
extern int	fstat __P ((int __fildes, struct stat *__stat_buf));

extern int	__mkdir __P ((__const char *__path, mode_t __mode));
extern int	mkdir __P ((__const char *__path, mode_t __mode));

extern int	__mknod __P ((__const char *__path, mode_t __mode,
			dev_t __dev));
extern int	mknod __P ((__const char *__path, mode_t __mode,
			dev_t __dev));

extern int	mkfifo __P ((__const char *__path, mode_t __mode));

extern int	__stat __P ((__const char *__filename,
			struct stat *__stat_buf));
extern int	stat __P ((__const char *__filename,
			struct stat *__stat_buf));

extern int	__lstat __P ((__const char *__filename,
			struct stat *__stat_buf));
extern int	lstat __P ((__const char *__filename,
			struct stat *__stat_buf));

extern mode_t	__umask __P ((mode_t __mask));
extern mode_t	umask __P ((mode_t __mask));

__END_DECLS

#endif
