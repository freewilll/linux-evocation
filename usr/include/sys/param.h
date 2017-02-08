#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H

#include <limits.h>
#include <linux/param.h>

/* Don't change it. H.J. */
#ifdef OLD_LINUX
#undef	MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 	8	/* max length of hostname */
#endif

#ifndef howmany
#define howmany(x, y)	(((x)+((y)-1))/(y))
#endif

#ifndef roundup
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#endif

#define MAXPATHLEN      PATH_MAX
#define NOFILE          OPEN_MAX

#endif
