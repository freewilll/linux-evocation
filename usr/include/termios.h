#ifndef _TERMIOS_H
#define _TERMIOS_H

#include <features.h>
#include <sys/types.h>
#include <linux/termios.h>

__BEGIN_DECLS

extern speed_t cfgetispeed __P ((struct termios *__termios_p));
extern speed_t cfgetospeed __P ((struct termios *__termios_p));

/* Set *T to indicate raw mode.  */
extern void cfmakeraw  __P ((struct termios *__t));

extern int cfsetispeed __P ((struct termios *__termios_p,
		speed_t __speed));
extern int cfsetospeed __P ((struct termios *__termios_p,
		speed_t __speed));

extern int tcdrain __P ((int __fildes));
extern int tcflow __P ((int __fildes, int __action));
extern int tcflush __P ((int __fildes, int __queue_selector));
extern pid_t tcgetpgrp __P ((int __fildes));
extern int tcgetattr __P ((int __fildes, struct termios *__termios_p));
extern int __tcgetattr __P ((int __fildes, struct termios *__termios_p));
extern int tcsendbreak __P ((int __fildes, int __duration));
extern int tcsetattr __P ((int __fildes, int __optional_actions,
	struct termios *__termios_p));
extern int tcsetpgrp __P ((int __fildes, pid_t __pgrp_id));

__END_DECLS

#endif
