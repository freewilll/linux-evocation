#include <linux/types.h>

extern inline int memcmp(const void * cs,const void * ct,size_t count)
{
int	d0, d1, d2;
register int __res;
__asm__ __volatile__(
	"repe\n\t"
	"cmpsb\n\t"
	"je 1f\n\t"
	"sbbl %0,%0\n\t"
	"orb $1,%b0\n"
	"1:"
	:"=a" (__res), "=&S" (d0), "=&D" (d1), "=&c" (d2)
	:"0" (0), "1" (cs), "2" (ct), "3" (count));
return __res;
}

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

extern inline void * memcpy(void * to, const void * from, size_t n)
{
int d0, d1, d2;
__asm__ __volatile__(
	"rep ; movsl\n\t"
	"testb $2,%b4\n\t"
	"je 1f\n\t"
	"movsw\n"
	"1:\ttestb $1,%b4\n\t"
	"je 2f\n\t"
	"movsb\n"
	"2:"
	:"=&c" (d0), "=&D" (d1), "=&S" (d2)
	:"0" (n/4), "q" (n),"1" ((long) to),"2" ((long) from)
	: "memory");
return (to);
}

