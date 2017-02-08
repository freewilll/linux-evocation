#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#include <features.h>

#include <sys/ipc.h>
#include <linux/shm.h>

__BEGIN_DECLS

extern int shmctl __P((int __shmid, int __cmd, struct shmid_ds *__buf));
extern int shmget __P((key_t __key, int __size, int __flag));
extern char *shmat __P((int __shmid, char *__shmaddr, int __shmflg));
extern int shmdt __P((char *__shmaddr));

__END_DECLS

#endif /* _SYS_SHM_H */
