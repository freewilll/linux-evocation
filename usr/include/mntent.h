#ifndef _MNTENT_H
#define _MNTENT_H

#include <features.h>

#define MNTTAB "/etc/fstab"
#define MOUNTED "/etc/mtab"

#define	MNTMAXSTR	128

#define	MNTTYPE_SWAP	"swap"		/* Swap device */
#define	MNTTYPE_IGNORE	"ignore"	/* Ignore this entry */

/* generic mount options */
#define	MNTOPT_DEFAULTS	"defaults"	/* use all default opts */
#define	MNTOPT_RO	"ro"		/* read only */
#define	MNTOPT_RW	"rw"		/* read/write */
#define	MNTOPT_SUID	"suid"		/* set uid allowed */
#define	MNTOPT_NOSUID	"nosuid"	/* no set uid allowed */
#define MNTOPT_NOAUTO	"noauto"	/* don't auto mount */

/* minix specific options */
/* none defined yet */

/* swap specific options */
/* none defined yet */

__BEGIN_DECLS

struct mntent{
       char  *mnt_fsname;
       char  *mnt_dir;
       char  *mnt_type;
       char  *mnt_opts;
       int   mnt_freq;
       int   mnt_passno;
};

__END_DECLS

#define __need_file
#include <stdio.h>

__BEGIN_DECLS

extern FILE	 *setmntent __P ((__const char *__filep,
			__const char *__type));
extern struct mntent
		*getmntent __P ((FILE *__filep));
extern int	addmntent __P ((FILE *__filep,
			__const struct mntent *__mnt));
extern char	*hasmntopt __P ((__const struct mntent *__mnt,
			__const char *__opt));
extern int	endmntent __P ((FILE *__filep));

__END_DECLS

#endif /* _MNTENT_H */
