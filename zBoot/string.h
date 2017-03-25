#ifndef BOOT_STRING_H
#define BOOT_STRING_H

#include <linux/types.h>	/* for size_t */

extern void * memcpy(void * to, const void * from, size_t n);
extern void * memset(void * s,char c,size_t count);
extern int memcmp(const void * cs,const void * ct,size_t count);

#endif /* BOOT_STRING_H */

