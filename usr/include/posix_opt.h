#ifndef _POSIX_OPT_H
#define _POSIX_OPT_H

/* only root can do a chown (I think..) */
#define _POSIX_CHOWN_RESTRICTED	1

/* no pathname truncation (but see in kernel) */
#define _POSIX_NO_TRUNC		1

/* character to disable things like ^C */
#define _POSIX_VDISABLE		'\0'

#define _POSIX_JOB_CONTROL	1

/* Implemented, for whatever good it is */
#define _POSIX_SAVED_IDS	1 

#endif
