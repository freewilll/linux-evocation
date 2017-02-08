#ifndef _FCNTL_H
#define _FCNTL_H

#include <features.h>
#include <sys/types.h>
#include <linux/fcntl.h>

__BEGIN_DECLS

extern int creat __P ((__const char * __filename, mode_t __mode));
extern int fcntl __P ((int __fildes,int __cmd, ...));
extern int __fcntl __P ((int __fildes,int __cmd, ...));
extern int open __P ((__const char * __filename, int __flags, ...));
extern int __open __P ((__const char * __filename, int __flags, ...));

__END_DECLS

#endif
