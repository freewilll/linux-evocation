#ifndef _BSD_NETINET_IN_H
#define _BSD_NETINET_IN_H

#include_next <netinet/in.h>

/* not implemented yet */

#undef IP_OPTIONS

/* concession to stupidity */

#define sin_zero __pad

#endif /* _BSD_NETINET_IN_H */
