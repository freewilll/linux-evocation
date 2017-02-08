/*
 * sys/mtio.h header file for Linux. Written by H. Bergman.
 * Split into <sys/mtio.h> and <linux/mtio.h> by K. Makisara.
 */

#ifndef _SYS_MTIO_H
#define _SYS_MTIO_H

#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/mtio.h>

#ifndef DEFTAPE
#define DEFTAPE "/dev/tape"	/* default tape device */
#endif /* DEFTAPE */

#endif /* not _SYS_MTIO_H */
