#ifndef _SYS_PTRACE_H
#define _SYS_PTRACE_H

#include <features.h>
#include <linux/ptrace.h>

__BEGIN_DECLS

extern int	ptrace __P ((int __request, int __pid, int __addr,
			int __data));

__END_DECLS

#endif /* _SYS_PTRACE_H */
