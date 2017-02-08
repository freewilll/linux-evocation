#ifndef _ULIMIT_H
#define _ULIMIT_H

#include <features.h>

#define	UL_GETFSIZE	1
#define	UL_SETFSIZE	2
	 
__BEGIN_DECLS

extern long ulimit __P((int __cmd, ...));

__END_DECLS

#endif /*_ULIMIT_H */
