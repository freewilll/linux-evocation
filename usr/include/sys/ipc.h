#ifndef _SYS_IPC_H
#define _SYS_IPC_H

#include <features.h>

#include <linux/ipc.h>

__BEGIN_DECLS

extern key_t ftok __P((char *__pathname, char __proc_id));

__END_DECLS

#endif /* _SYS_IPC_H */
