#ifndef _TIMES_H
#define _TIMES_H

#include <features.h>
#include <sys/types.h>
#include <time.h>
#include <linux/times.h>

__BEGIN_DECLS

extern clock_t times __P ((struct tms * __tp));
extern clock_t __times __P ((struct tms * __tp));

__END_DECLS

#endif
