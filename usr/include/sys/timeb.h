#ifndef _SYS_TIMEB_H
#define _SYS_TIMEB_H

#include <features.h>
#include <sys/types.h>

struct timeb {
	time_t   time;
	unsigned short millitm;
	short    timezone;
	short    dstflag;
};


__BEGIN_DECLS

extern int      ftime __P ((struct timeb *__tp));

__END_DECLS

#endif /* _SYS_TIMEB_H */
