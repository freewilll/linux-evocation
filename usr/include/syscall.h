#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <features.h>
#include <sys/syscall.h>
#include <errno.h>

#ifndef __MAX_ERRNO
#define __MAX_ERRNO 4096
#endif

/* In case of returning a memory address, negative values may
 * not mean error.
 */
#ifdef __CHECK_RETURN_ADDR
#define __check_errno(__res)	\
	((__res) >= 0 || (__res) < -__MAX_ERRNO)
#else
#define __check_errno(__res)	((__res) >= 0)
#endif

__BEGIN_DECLS

extern	int syscall __P ((int __number, ...));

__END_DECLS

#define _syscall0(type,name) \
type name(void) \
{ \
long __res; \
__asm__ __volatile__ ("int $0x80" \
	: "=a" (__res) \
	: "0" (SYS_##name)); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#if defined(__PIC__) || defined (__pic__)
#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ \
long __res; \
__asm__ __volatile__ ("pushl %%ebx\n\t" \
		  "movl %%ecx,%%ebx\n\t" \
		  "int $0x80\n\t" \
		  "popl %%ebx" \
	: "=a" (__res) \
	: "0" (SYS_##name),"c" ((long)(a)):"bx"); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
long __res; \
__asm__ __volatile__ ("pushl %%ebx\n\t" \
		  "movl %%edx,%%ebx\n\t" \
		  "int $0x80\n\t" \
		  "popl %%ebx" \
	: "=a" (__res) \
	: "0" (SYS_##name),"d" ((long)(a)),"c" ((long)(b)):"bx"); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
long __res; \
__asm__ __volatile__ ("pushl %%ebx\n\t" \
		  "movl %%esi,%%ebx\n\t" \
		  "int $0x80\n\t" \
		  "popl %%ebx" \
	: "=a" (__res) \
	: "0" (SYS_##name),"S" ((long)(a)),"c" ((long)(b)),"d" ((long)(c)):"bx"); \
if (__check_errno(__res)) \
	return (type) __res; \
errno=-__res; \
return -1; \
}

#define _syscall4(type,name,atype,a,btype,b,ctype,c,dtype,d) \
type name (atype a, btype b, ctype c, dtype d) \
{ \
long __res; \
__asm__ __volatile__ ("pushl %%ebx\n\t" \
		  "movl %%edi,%%ebx\n\t" \
		  "int $0x80\n\t" \
		  "popl %%ebx" \
	: "=a" (__res) \
	: "0" (SYS_##name),"D" ((long)(a)),"c" ((long)(b)), \
	  "d" ((long)(c)),"S" ((long)(d))); \
if (__check_errno(__res)) \
	return (type) __res; \
errno=-__res; \
return -1; \
}

#else	/* PIC */

#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ \
long __res; \
__asm__ __volatile__ ("int $0x80" \
	: "=a" (__res) \
	: "0" (SYS_##name),"b" ((long)(a)):"bx"); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
long __res; \
__asm__ __volatile__ ("int $0x80" \
	: "=a" (__res) \
	: "0" (SYS_##name),"b" ((long)(a)),"c" ((long)(b)):"bx"); \
if (__check_errno(__res)) \
	return (type) __res; \
errno = -__res; \
return -1; \
}

#define _syscall3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
long __res; \
__asm__ __volatile__ ("int $0x80" \
	: "=a" (__res) \
	: "0" (SYS_##name),"b" ((long)(a)),"c" ((long)(b)),"d" ((long)(c)):"bx"); \
if (__check_errno(__res)) \
	return (type) __res; \
errno=-__res; \
return -1; \
}

#define _syscall4(type,name,atype,a,btype,b,ctype,c,dtype,d) \
type name (atype a, btype b, ctype c, dtype d) \
{ \
long __res; \
__asm__ __volatile__ ("int $0x80" \
	: "=a" (__res) \
	: "0" (SYS_##name),"b" ((long)(a)),"c" ((long)(b)), \
	  "d" ((long)(c)),"S" ((long)(d))); \
if (__check_errno(__res)) \
	return (type) __res; \
errno=-__res; \
return -1; \
}

#define _syscall5(type,name,atype,a,btype,b,ctype,c,dtype,d,etype,e) \
type name (atype a,btype b,ctype c,dtype d,etype e) \
{ \
long __res; \
__asm__ __volatile__ ("int $0x80" \
	: "=a" (__res) \
	: "0" (SYS_##name),"b" ((long)(a)),"c" ((long)(b)), \
	  "d" ((long)(c)),"S" ((long)(d)),"D" ((long)(e))); \
if (__check_errno(__res)) \
	return (type) __res; \
errno=-__res; \
return -1; \
}

#endif	/* PIC */

#endif
