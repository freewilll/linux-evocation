#ifndef _SYS_SEM_H
#define _SYS_SEM_H

#include <features.h>

#include <sys/ipc.h>
#include <linux/sem.h>
#include <asm/bitops.h>

/*
 * <asm/bitops.h>:
 * set_bit, clear_bit, test_bit
 * all return the value of the bit prior to alteration.
 * It seems they are more useful than sysv sems if you 
 * dont have a need for semaphore arrays.
 */

__BEGIN_DECLS

extern int semget __P((key_t __key, int __nsems, int __semflg));
extern int semop __P((int __semid, struct sembuf *__sops, unsigned __nsops));
extern int semctl __P((int __semid, int __semnum, int __cmd, 
		       union semun __arg));

__END_DECLS

#endif /* _SYS_SEM_H */
