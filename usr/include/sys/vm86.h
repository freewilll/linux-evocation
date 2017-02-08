#ifndef _SYS_VM86_H
#define _SYS_VM86_H

#include <features.h>
#include <linux/vm86.h>

__BEGIN_DECLS

extern vm86(struct vm86_struct * __info);

__END_DECLS

#endif /*_SYS_VM86_H */
