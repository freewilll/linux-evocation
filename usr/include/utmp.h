/* utmp.h for Linux, by poe@daimi.aau.dk */

#ifndef UTMP_H
#define UTMP_H

#include <features.h>
#include <sys/types.h>
#include <time.h>

#define UTMP_FILE	"/etc/utmp"
#define WTMP_FILE	"/usr/adm/wtmp"

#define UTMP_FILENAME	UTMP_FILE
#define WTMP_FILENAME	WTMP_FILE

/* defines for the ut_type field */
#define UT_UNKNOWN	0

/* size of user name */
#define UT_NAMESIZE     8

#define RUN_LVL		1
#define BOOT_TIME	2
#define NEW_TIME	3
#define OLD_TIME	4

#define INIT_PROCESS	5
#define LOGIN_PROCESS	6
#define USER_PROCESS	7
#define DEAD_PROCESS	8


struct utmp {
	short	ut_type;	/* type of login */
	pid_t	ut_pid;		/* pid of login-process */
	char	ut_line[12];	/* devicename of tty -"/dev/", null-term */
	char	ut_id[2];	/* abbrev. ttyname, as 01, s1 etc. */
	time_t	ut_time;	/* logintime */
	char	ut_user[UT_NAMESIZE];	/* username, not null-term */
	char	ut_host[16];	/* hostname for remote login... */
	long	ut_addr;	/* IP addr of remote host */
};

#define ut_name ut_user

__BEGIN_DECLS

extern void		setutent __P ((void));
extern void		utmpname __P ((__const char *));
extern struct utmp	*getutent __P ((void));
extern struct utmp	*getutid __P ((struct utmp *));
extern struct utmp 	*getutline __P ((struct utmp *));
extern void		pututline __P ((struct utmp *));
extern struct utmp	*_getutline __P ((struct utmp *));
extern void		endutent __P ((void));

__END_DECLS

#endif	
