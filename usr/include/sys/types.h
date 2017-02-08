#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <linux/types.h>

#ifndef howmany
#define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#ifndef roundup
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#endif

/* Don't change it without checking <linux/types.h>. */
typedef unsigned long fd_mask;

#endif /* _SYS_TYPES_H */
