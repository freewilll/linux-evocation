/*
 * linux/ipc/util.c
 * Copyright (C) 1992 Krishna Balasubramanian
 */

#include <linux/config.h>
#include <linux/errno.h>
#include <asm/segment.h>
#include <linux/sched.h>
// TODO WGJA WIP: #include <linux/sem.h>
// TODO WGJA WIP: #include <linux/msg.h>
// TODO WGJA WIP: #include <linux/shm.h>
#include <linux/stat.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: void ipc_init (void);
// TODO WGJA WIP: extern "C" int sys_ipc (uint call, int first, int second, int third, void *ptr); 
// TODO WGJA WIP: 
#ifdef CONFIG_SYSVIPC

// TODO WGJA WIP: int ipcperms (struct ipc_perm *ipcp, short flag);
// TODO WGJA WIP: extern void sem_init (void), msg_init (void), shm_init (void);
// TODO WGJA WIP: extern int sys_semget (key_t key, int nsems, int semflg);
// TODO WGJA WIP: extern int sys_semop (int semid, struct sembuf *sops, unsigned nsops);
// TODO WGJA WIP: extern int sys_semctl (int semid, int semnum, int cmd, void *arg);
// TODO WGJA WIP: extern int sys_msgget (key_t key, int msgflg);
// TODO WGJA WIP: extern int sys_msgsnd (int msqid, struct msgbuf *msgp, int msgsz, int msgflg);
// TODO WGJA WIP: extern int sys_msgrcv (int msqid, struct msgbuf *msgp, int msgsz, long msgtyp,
// TODO WGJA WIP: 		       int msgflg);
// TODO WGJA WIP: extern int sys_msgctl (int msqid, int cmd, struct msqid_ds *buf);
// TODO WGJA WIP: extern int sys_shmctl (int shmid, int cmd, struct shmid_ds *buf);
// TODO WGJA WIP: extern int sys_shmget (key_t key, int size, int flag);
// TODO WGJA WIP: extern int sys_shmat (int shmid, char *shmaddr, int shmflg, ulong *addr);
// TODO WGJA WIP: extern int sys_shmdt (char *shmaddr);
// TODO WGJA WIP: 
// TODO WGJA WIP: void ipc_init (void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	sem_init();
// TODO WGJA WIP: 	msg_init();
// TODO WGJA WIP: 	shm_init();
// TODO WGJA WIP: 	return;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /* 
// TODO WGJA WIP:  * Check user, group, other permissions for access
// TODO WGJA WIP:  * to ipc resources. return 0 if allowed
// TODO WGJA WIP:  */
// TODO WGJA WIP: int ipcperms (struct ipc_perm *ipcp, short flag)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i; mode_t perm; uid_t euid; int egid;
	// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (suser())
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	perm = S_IRWXO; euid = current->euid;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (euid == ipcp->cuid || euid == ipcp->uid) 
// TODO WGJA WIP: 		perm = S_IRWXU;
// TODO WGJA WIP: 	else {
// TODO WGJA WIP: 		for (i = 0; (egid = current->groups[i]) != NOGROUP; i++)
// TODO WGJA WIP: 			if ((egid == ipcp->cgid) || (egid == ipcp->gid)) { 
// TODO WGJA WIP: 				perm = S_IRWXG; 
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!(flag & perm) || flag & perm & ~ipcp->mode)
// TODO WGJA WIP: 		return -1;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_ipc (uint call, int first, int second, int third, void *ptr) 
// TODO WGJA WIP: {
	// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (call <= SEMCTL)
// TODO WGJA WIP: 		switch (call) {
// TODO WGJA WIP: 		case SEMOP:
// TODO WGJA WIP: 			return sys_semop (first, (struct sembuf *)ptr, second);
// TODO WGJA WIP: 		case SEMGET:
// TODO WGJA WIP: 			return sys_semget (first, second, third);
// TODO WGJA WIP: 		case SEMCTL:
// TODO WGJA WIP: 			return sys_semctl (first, second, third, ptr);
// TODO WGJA WIP: 		default:
// TODO WGJA WIP: 			return -EINVAL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	if (call <= MSGCTL) 
// TODO WGJA WIP: 		switch (call) {
// TODO WGJA WIP: 		case MSGSND:
// TODO WGJA WIP: 			return sys_msgsnd (first, (struct msgbuf *) ptr, 
// TODO WGJA WIP: 					   second, third);
// TODO WGJA WIP: 		case MSGRCV: {
// TODO WGJA WIP: 			struct ipc_kludge tmp; 
// TODO WGJA WIP: 			if (!ptr)
// TODO WGJA WIP: 				return -EINVAL;
// TODO WGJA WIP: 			memcpy_fromfs (&tmp,(struct ipc_kludge *) ptr, 
// TODO WGJA WIP: 				       sizeof (tmp));
// TODO WGJA WIP: 			return sys_msgrcv (first, tmp.msgp, second, tmp.msgtyp,
// TODO WGJA WIP: 					 	third);
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		case MSGGET:
// TODO WGJA WIP: 			return sys_msgget ((key_t) first, second);
// TODO WGJA WIP: 		case MSGCTL:
// TODO WGJA WIP: 			return sys_msgctl (first, second, 
// TODO WGJA WIP: 						(struct msqid_ds *) ptr);
// TODO WGJA WIP: 		default:
// TODO WGJA WIP: 			return -EINVAL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	if (call <= SHMCTL) 
// TODO WGJA WIP: 		switch (call) {
// TODO WGJA WIP: 		case SHMAT: /* returning shmaddr > 2G will screw up */
// TODO WGJA WIP: 			return sys_shmat (first, (char *) ptr, second, 
// TODO WGJA WIP: 							(ulong *) third);
// TODO WGJA WIP: 		case SHMDT: 
// TODO WGJA WIP: 			return sys_shmdt ((char *)ptr);
// TODO WGJA WIP: 		case SHMGET:
// TODO WGJA WIP: 			return sys_shmget (first, second, third);
// TODO WGJA WIP: 		case SHMCTL:
// TODO WGJA WIP: 			return sys_shmctl (first, second, 
// TODO WGJA WIP: 						(struct shmid_ds *) ptr);
// TODO WGJA WIP: 		default:
// TODO WGJA WIP: 			return -EINVAL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	return -EINVAL;
// TODO WGJA WIP: }

#else /* not CONFIG_SYSVIPC */

// TODO WGJA WIP: extern "C" int sys_ipc (uint call, int first, int second, int third, void *ptr) 
// TODO WGJA WIP: {
// TODO WGJA WIP:     return -ENOSYS;
// TODO WGJA WIP: }

int shm_fork (struct task_struct *p1, struct task_struct *p2)
{
    return 0;
}

void sem_exit (void)
{
    return;
}

void shm_exit (void)
{
    return;
}

int shm_swap (int prio)
{
    return 0;
}

void shm_no_page (unsigned long *ptent)
{
    return;
}

#endif
