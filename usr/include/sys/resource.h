/*
 * Resource control/accounting header file for linux
 */

#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <features.h>
#include <sys/time.h>
#include <linux/resource.h>

__BEGIN_DECLS

extern int	getrlimit __P ((int __resource,
			struct rlimit *__rlp));
extern int	setrlimit __P ((int __resource,
			__const struct rlimit *__rlp));

extern int      getpriority __P((int __which, int __who));
extern int      setpriority __P((int __which, int __who,
			int __prio));

extern int	__getrusage __P ((int __who, struct rusage *__rusage));
extern int	getrusage __P ((int __who, struct rusage *__rusage));

__END_DECLS

#endif /* _SYS_RESOURCE_H */
