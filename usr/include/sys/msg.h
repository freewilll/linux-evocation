#ifndef _SYS_MSG_H
#define _SYS_MSG_H

#include <features.h>

#include <sys/ipc.h>
#include <linux/msg.h>

__BEGIN_DECLS

extern int msgget __P((key_t __key, int __msgflg));
extern int msgsnd __P((int __msqid, struct msgbuf *__msgp, int __msgsz, 
		       int __msgflg));
extern int msgrcv __P((int __msqid, struct msgbuf *__msgp, int __msgsz,
		       long __msgtyp, int __msgflg));
extern int msgctl __P((int __msqid, int __cmd, struct msqid_ds *__buf));

__END_DECLS

#endif /* _SYS_MSG_H */
