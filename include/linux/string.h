#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

#include <linux/types.h>	/* for size_t */

// TODO WGJA WIP: #ifndef NULL
// TODO WGJA WIP: #define NULL ((void *) 0)
// TODO WGJA WIP: #endif

/*
 * This string-include defines all string functions as inline
 * functions. Use gcc. It also assumes ds=es=data space, this should be
 * normal. Most of the string-functions are rather heavily hand-optimized,
 * see especially strtok,strstr,str[c]spn. They should work, but are not
 * very easy to understand. Everything is done entirely within the register
 * set, making the functions fast and clean. String instructions have been
 * used through-out, making for "slightly" unclear code :-)
 *
 *		Copyright (C) 1991, 1992 Linus Torvalds
 */

extern inline char * strcpy(char * dest,const char *src)
{
	int d0, d1, d2;
	asm volatile("1:\tlodsb\n\t"
		"stosb\n\t"
		"testb %%al,%%al\n\t"
		"jne 1b"
		: "=&S" (d0), "=&D" (d1), "=&a" (d2)
		: "0" (src), "1" (dest) : "memory");
	return dest;
}

// TODO WGJA WIP: extern inline char * strncpy(char * dest,const char *src,size_t count)
// TODO WGJA WIP: {
// TODO WGJA WIP: __asm__("cld\n"
// TODO WGJA WIP: 	"1:\tdecl %2\n\t"
// TODO WGJA WIP: 	"js 2f\n\t"
// TODO WGJA WIP: 	"lodsb\n\t"
// TODO WGJA WIP: 	"stosb\n\t"
// TODO WGJA WIP: 	"testb %%al,%%al\n\t"
// TODO WGJA WIP: 	"jne 1b\n\t"
// TODO WGJA WIP: 	"rep\n\t"
// TODO WGJA WIP: 	"stosb\n"
// TODO WGJA WIP: 	"2:"
// TODO WGJA WIP: 	: /* no output */
// TODO WGJA WIP: 	:"S" (src),"D" (dest),"c" (count):"si","di","ax","cx","memory");
// TODO WGJA WIP: return dest;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline char * strcat(char * dest,const char * src)
// TODO WGJA WIP: {
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"decl %1\n"
// TODO WGJA WIP: 	"1:\tlodsb\n\t"
// TODO WGJA WIP: 	"stosb\n\t"
// TODO WGJA WIP: 	"testb %%al,%%al\n\t"
// TODO WGJA WIP: 	"jne 1b"
// TODO WGJA WIP: 	: /* no output */
// TODO WGJA WIP: 	:"S" (src),"D" (dest),"a" (0),"c" (0xffffffff):"si","di","ax","cx");
// TODO WGJA WIP: return dest;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline char * strncat(char * dest,const char * src,size_t count)
// TODO WGJA WIP: {
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"decl %1\n\t"
// TODO WGJA WIP: 	"movl %4,%3\n"
// TODO WGJA WIP: 	"1:\tdecl %3\n\t"
// TODO WGJA WIP: 	"js 2f\n\t"
// TODO WGJA WIP: 	"lodsb\n\t"
// TODO WGJA WIP: 	"stosb\n\t"
// TODO WGJA WIP: 	"testb %%al,%%al\n\t"
// TODO WGJA WIP: 	"jne 1b\n"
// TODO WGJA WIP: 	"2:\txorl %2,%2\n\t"
// TODO WGJA WIP: 	"stosb"
// TODO WGJA WIP: 	: /* no output */
// TODO WGJA WIP: 	:"S" (src),"D" (dest),"a" (0),"c" (0xffffffff),"g" (count)
// TODO WGJA WIP: 	:"si","di","ax","cx","memory");
// TODO WGJA WIP: return dest;
// TODO WGJA WIP: }

static inline int strcmp(const char * cs,const char * ct)
{
int d0, d1;
register int __res;
__asm__ __volatile__(
	"1:\tlodsb\n\t"
	"scasb\n\t"
	"jne 2f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"xorl %%eax,%%eax\n\t"
	"jmp 3f\n"
	"2:\tsbbl %%eax,%%eax\n\t"
	"orb $1,%%al\n"
	"3:"
	:"=a" (__res), "=&S" (d0), "=&D" (d1)
		     :"1" (cs),"2" (ct));
return __res;
}

static inline int strncmp(const char * cs,const char * ct,size_t count)
{
register int __res;
int d0, d1, d2;
__asm__ __volatile__(
	"1:\tdecl %3\n\t"
	"js 2f\n\t"
	"lodsb\n\t"
	"scasb\n\t"
	"jne 3f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n"
	"2:\txorl %%eax,%%eax\n\t"
	"jmp 4f\n"
	"3:\tsbbl %%eax,%%eax\n\t"
	"orb $1,%%al\n"
	"4:"
		     :"=a" (__res), "=&S" (d0), "=&D" (d1), "=&c" (d2)
		     :"1" (cs),"2" (ct),"3" (count));
return __res;
}

static inline char * strchr(const char * s, int c)
{
int d0;
register char * __res;
__asm__ __volatile__(
	"movb %%al,%%ah\n"
	"1:\tlodsb\n\t"
	"cmpb %%ah,%%al\n\t"
	"je 2f\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b\n\t"
	"movl $1,%1\n"
	"2:\tmovl %1,%0\n\t"
	"decl %0"
	:"=a" (__res), "=&S" (d0) : "1" (s),"0" (c));
return __res;
}

// TODO WGJA WIP: extern inline char * strrchr(const char * s,char c)
// TODO WGJA WIP: {
// TODO WGJA WIP: register char * __res __asm__("dx");
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"movb %%al,%%ah\n"
// TODO WGJA WIP: 	"1:\tlodsb\n\t"
// TODO WGJA WIP: 	"cmpb %%ah,%%al\n\t"
// TODO WGJA WIP: 	"jne 2f\n\t"
// TODO WGJA WIP: 	"movl %%esi,%0\n\t"
// TODO WGJA WIP: 	"decl %0\n"
// TODO WGJA WIP: 	"2:\ttestb %%al,%%al\n\t"
// TODO WGJA WIP: 	"jne 1b"
// TODO WGJA WIP: 	:"=d" (__res):"0" (0),"S" (s),"a" (c):"ax","si");
// TODO WGJA WIP: return __res;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline size_t strspn(const char * cs, const char * ct)
// TODO WGJA WIP: {
// TODO WGJA WIP: register char * __res __asm__("si");
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"notl %%ecx\n\t"
// TODO WGJA WIP: 	"decl %%ecx\n\t"
// TODO WGJA WIP: 	"movl %%ecx,%%edx\n"
// TODO WGJA WIP: 	"1:\tlodsb\n\t"
// TODO WGJA WIP: 	"testb %%al,%%al\n\t"
// TODO WGJA WIP: 	"je 2f\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"movl %%edx,%%ecx\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"je 1b\n"
// TODO WGJA WIP: 	"2:\tdecl %0"
// TODO WGJA WIP: 	:"=S" (__res):"a" (0),"c" (0xffffffff),"0" (cs),"g" (ct)
// TODO WGJA WIP: 	:"ax","cx","dx","di");
// TODO WGJA WIP: return __res-cs;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline size_t strcspn(const char * cs, const char * ct)
// TODO WGJA WIP: {
// TODO WGJA WIP: register char * __res __asm__("si");
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"notl %%ecx\n\t"
// TODO WGJA WIP: 	"decl %%ecx\n\t"
// TODO WGJA WIP: 	"movl %%ecx,%%edx\n"
// TODO WGJA WIP: 	"1:\tlodsb\n\t"
// TODO WGJA WIP: 	"testb %%al,%%al\n\t"
// TODO WGJA WIP: 	"je 2f\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"movl %%edx,%%ecx\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"jne 1b\n"
// TODO WGJA WIP: 	"2:\tdecl %0"
// TODO WGJA WIP: 	:"=S" (__res):"a" (0),"c" (0xffffffff),"0" (cs),"g" (ct)
// TODO WGJA WIP: 	:"ax","cx","dx","di");
// TODO WGJA WIP: return __res-cs;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline char * strpbrk(const char * cs,const char * ct)
// TODO WGJA WIP: {
// TODO WGJA WIP: register char * __res __asm__("si");
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"notl %%ecx\n\t"
// TODO WGJA WIP: 	"decl %%ecx\n\t"
// TODO WGJA WIP: 	"movl %%ecx,%%edx\n"
// TODO WGJA WIP: 	"1:\tlodsb\n\t"
// TODO WGJA WIP: 	"testb %%al,%%al\n\t"
// TODO WGJA WIP: 	"je 2f\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"movl %%edx,%%ecx\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"jne 1b\n\t"
// TODO WGJA WIP: 	"decl %0\n\t"
// TODO WGJA WIP: 	"jmp 3f\n"
// TODO WGJA WIP: 	"2:\txorl %0,%0\n"
// TODO WGJA WIP: 	"3:"
// TODO WGJA WIP: 	:"=S" (__res):"a" (0),"c" (0xffffffff),"0" (cs),"g" (ct)
// TODO WGJA WIP: 	:"ax","cx","dx","di");
// TODO WGJA WIP: return __res;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline char * strstr(const char * cs,const char * ct)
// TODO WGJA WIP: {
// TODO WGJA WIP: register char * __res __asm__("ax");
// TODO WGJA WIP: __asm__("cld\n\t" \
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"notl %%ecx\n\t"
// TODO WGJA WIP: 	"decl %%ecx\n\t"	/* NOTE! This also sets Z if searchstring='' */
// TODO WGJA WIP: 	"movl %%ecx,%%edx\n"
// TODO WGJA WIP: 	"1:\tmovl %4,%%edi\n\t"
// TODO WGJA WIP: 	"movl %%esi,%%eax\n\t"
// TODO WGJA WIP: 	"movl %%edx,%%ecx\n\t"
// TODO WGJA WIP: 	"repe\n\t"
// TODO WGJA WIP: 	"cmpsb\n\t"
// TODO WGJA WIP: 	"je 2f\n\t"		/* also works for empty string, see above */
// TODO WGJA WIP: 	"xchgl %%eax,%%esi\n\t"
// TODO WGJA WIP: 	"incl %%esi\n\t"
// TODO WGJA WIP: 	"cmpb $0,-1(%%eax)\n\t"
// TODO WGJA WIP: 	"jne 1b\n\t"
// TODO WGJA WIP: 	"xorl %%eax,%%eax\n\t"
// TODO WGJA WIP: 	"2:"
// TODO WGJA WIP: 	:"=a" (__res):"0" (0),"c" (0xffffffff),"S" (cs),"g" (ct)
// TODO WGJA WIP: 	:"cx","dx","di","si");
// TODO WGJA WIP: return __res;
// TODO WGJA WIP: }

static inline size_t strlen(const char * s)
{
int d0;
register int __res;
__asm__ __volatile__(
	"repne\n\t"
	"scasb\n\t"
	"notl %0\n\t"
	"decl %0"
	:"=c" (__res), "=&D" (d0) :"1" (s),"a" (0), "0" (0xffffffff));
return __res;
}

// TODO WGJA WIP: extern char * ___strtok;
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline char * strtok(char * s,const char * ct)
// TODO WGJA WIP: {
// TODO WGJA WIP: register char * __res;
// TODO WGJA WIP: __asm__("testl %1,%1\n\t"
// TODO WGJA WIP: 	"jne 1f\n\t"
// TODO WGJA WIP: 	"testl %0,%0\n\t"
// TODO WGJA WIP: 	"je 8f\n\t"
// TODO WGJA WIP: 	"movl %0,%1\n"
// TODO WGJA WIP: 	"1:\txorl %0,%0\n\t"
// TODO WGJA WIP: 	"movl $-1,%%ecx\n\t"
// TODO WGJA WIP: 	"xorl %%eax,%%eax\n\t"
// TODO WGJA WIP: 	"cld\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"notl %%ecx\n\t"
// TODO WGJA WIP: 	"decl %%ecx\n\t"
// TODO WGJA WIP: 	"je 7f\n\t"			/* empty delimeter-string */
// TODO WGJA WIP: 	"movl %%ecx,%%edx\n"
// TODO WGJA WIP: 	"2:\tlodsb\n\t"
// TODO WGJA WIP: 	"testb %%al,%%al\n\t"
// TODO WGJA WIP: 	"je 7f\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"movl %%edx,%%ecx\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"je 2b\n\t"
// TODO WGJA WIP: 	"decl %1\n\t"
// TODO WGJA WIP: 	"cmpb $0,(%1)\n\t"
// TODO WGJA WIP: 	"je 7f\n\t"
// TODO WGJA WIP: 	"movl %1,%0\n"
// TODO WGJA WIP: 	"3:\tlodsb\n\t"
// TODO WGJA WIP: 	"testb %%al,%%al\n\t"
// TODO WGJA WIP: 	"je 5f\n\t"
// TODO WGJA WIP: 	"movl %4,%%edi\n\t"
// TODO WGJA WIP: 	"movl %%edx,%%ecx\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"jne 3b\n\t"
// TODO WGJA WIP: 	"decl %1\n\t"
// TODO WGJA WIP: 	"cmpb $0,(%1)\n\t"
// TODO WGJA WIP: 	"je 5f\n\t"
// TODO WGJA WIP: 	"movb $0,(%1)\n\t"
// TODO WGJA WIP: 	"incl %1\n\t"
// TODO WGJA WIP: 	"jmp 6f\n"
// TODO WGJA WIP: 	"5:\txorl %1,%1\n"
// TODO WGJA WIP: 	"6:\tcmpb $0,(%0)\n\t"
// TODO WGJA WIP: 	"jne 7f\n\t"
// TODO WGJA WIP: 	"xorl %0,%0\n"
// TODO WGJA WIP: 	"7:\ttestl %0,%0\n\t"
// TODO WGJA WIP: 	"jne 8f\n\t"
// TODO WGJA WIP: 	"movl %0,%1\n"
// TODO WGJA WIP: 	"8:"
// TODO WGJA WIP: 	:"=b" (__res),"=S" (___strtok)
// TODO WGJA WIP: 	:"0" (___strtok),"1" (s),"g" (ct)
// TODO WGJA WIP: 	:"ax","cx","dx","di","memory");
// TODO WGJA WIP: return __res;
// TODO WGJA WIP: }

extern inline void * memcpy(void * to, const void * from, size_t n)
{
int d0, d1;
__asm__("cld\n\t"
	"movl %%edx, %%ecx\n\t"
	"shrl $2,%%ecx\n\t"
	"rep ; movsl\n\t"
	"testb $1,%%dl\n\t"
	"je 1f\n\t"
	"movsb\n"
	"1:\ttestb $2,%%dl\n\t"
	"je 2f\n\t"
	"movsw\n"
	"2:\n"
	: "=&D" (d0), "=&S" (d1)
	:"d" (n),"D" ((long) to),"S" ((long) from)
	: "cx","memory");

return (to);
}

// TODO WGJA WIP: extern inline void * memmove(void * dest,const void * src, size_t n)
// TODO WGJA WIP: {
// TODO WGJA WIP: if (dest<src)
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"rep\n\t"
// TODO WGJA WIP: 	"movsb"
// TODO WGJA WIP: 	: /* no output */
// TODO WGJA WIP: 	:"c" (n),"S" (src),"D" (dest)
// TODO WGJA WIP: 	:"cx","si","di");
// TODO WGJA WIP: else
// TODO WGJA WIP: __asm__("std\n\t"
// TODO WGJA WIP: 	"rep\n\t"
// TODO WGJA WIP: 	"movsb\n\t"
// TODO WGJA WIP: 	"cld"
// TODO WGJA WIP: 	: /* no output */
// TODO WGJA WIP: 	:"c" (n),"S" (src+n-1),"D" (dest+n-1)
// TODO WGJA WIP: 	:"cx","si","di","memory");
// TODO WGJA WIP: return dest;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline int memcmp(const void * cs,const void * ct,size_t count)
// TODO WGJA WIP: {
// TODO WGJA WIP: register int __res __asm__("ax");
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"repe\n\t"
// TODO WGJA WIP: 	"cmpsb\n\t"
// TODO WGJA WIP: 	"je 1f\n\t"
// TODO WGJA WIP: 	"movl $1,%%eax\n\t"
// TODO WGJA WIP: 	"jb 1f\n\t"
// TODO WGJA WIP: 	"negl %%eax\n"
// TODO WGJA WIP: 	"1:"
// TODO WGJA WIP: 	:"=a" (__res):"0" (0),"D" (cs),"S" (ct),"c" (count)
// TODO WGJA WIP: 	:"si","di","cx");
// TODO WGJA WIP: return __res;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline void * memchr(const void * cs,char c,size_t count)
// TODO WGJA WIP: {
// TODO WGJA WIP: register void * __res __asm__("di");
// TODO WGJA WIP: if (!count)
// TODO WGJA WIP: 	return NULL;
// TODO WGJA WIP: __asm__("cld\n\t"
// TODO WGJA WIP: 	"repne\n\t"
// TODO WGJA WIP: 	"scasb\n\t"
// TODO WGJA WIP: 	"je 1f\n\t"
// TODO WGJA WIP: 	"movl $1,%0\n"
// TODO WGJA WIP: 	"1:\tdecl %0"
// TODO WGJA WIP: 	:"=D" (__res):"a" (c),"D" (cs),"c" (count)
// TODO WGJA WIP: 	:"cx");
// TODO WGJA WIP: return __res;
// TODO WGJA WIP: }

extern inline void * memset(void * s,char c,size_t count)
{
int d0, d1;
__asm__ __volatile__(
	"cld\n\t"
	"rep\n\t"
	"stosb"
	: "=&c" (d0), "=&D" (d1)
	:"a" (c),"1" (s),"0" (count)
	:"memory");
return s;
}

#endif
