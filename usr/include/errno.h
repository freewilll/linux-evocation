#ifndef _ERRNO_H
#define _ERRNO_H

#include <features.h>
#include <linux/errno.h>

#ifdef  __USE_BSD
extern int sys_nerr;
extern char *sys_errlist[];
#endif
#ifdef  __USE_GNU
extern int _sys_nerr;
extern char *_sys_errlist[];
#endif

extern int	errno;

__BEGIN_DECLS

extern void	perror __P ((__const char* __s));
extern char*	strerr __P ((int __errno));

__END_DECLS

#endif
