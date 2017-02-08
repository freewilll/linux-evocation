#ifndef _UTIME_H
#define _UTIME_H

#include <features.h>
#include <sys/types.h>	/* I know - shouldn't do this, but .. */
#include <linux/utime.h>

__BEGIN_DECLS

extern int utime __P ((__const char *__filename,
		struct utimbuf *__times));

__END_DECLS

#endif
