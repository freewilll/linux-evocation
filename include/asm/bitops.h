#ifndef _ASM_BITOPS_H
#define _ASM_BITOPS_H
/*
 * Copyright 1992, Linus Torvalds.
 */

#ifdef i386
/*
 * These have to be done with inline assembly: that way the bit-setting
 * is guaranteed to be atomic. All bitoperations return 0 if the bit
 * was cleared before the operation and != 0 if it was not.
 *
 * bit 0 is the LSB of addr; bit 32 is the LSB of (addr+1).
 */

/*
 * Some hacks to defeat gcc over-optimizations..
 */
struct __dummy { unsigned long a[100]; };
#define ADDR (*(struct __dummy *) addr)

extern __inline__ int set_bit(int nr, void * addr)
{
	int oldbit;

	__asm__ __volatile__("btsl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit),"=m" (ADDR)
		:"r" (nr));
	return oldbit;
}

extern __inline__ int clear_bit(int nr, void * addr)
{
	int oldbit;

	__asm__ __volatile__("btrl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit),"=m" (ADDR)
		:"r" (nr));
	return oldbit;
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This routine doesn't need to be atomic, but it's faster to code it
// TODO WGJA WIP:  * this way.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern __inline__ int test_bit(int nr, void * addr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int oldbit;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	__asm__ __volatile__("btl %2,%1\n\tsbbl %0,%0"
// TODO WGJA WIP: 		:"=r" (oldbit)
// TODO WGJA WIP: 		:"m" (ADDR),"r" (nr));
// TODO WGJA WIP: 	return oldbit;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: #else
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * For the benefit of those who are trying to port Linux to another
// TODO WGJA WIP:  * architecture, here are some C-language equivalents.  You should
// TODO WGJA WIP:  * recode these in the native assmebly language, if at all possible.
// TODO WGJA WIP:  * To guarantee atomicity, these routines call cli() and sti() to
// TODO WGJA WIP:  * disable interrupts while they operate.  (You have to provide inline
// TODO WGJA WIP:  * routines to cli() and sti().)
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Also note, these routines assume that you have 32 bit integers.
// TODO WGJA WIP:  * You will have to change this if you are trying to port Linux to the
// TODO WGJA WIP:  * Alpha architecture or to a Cray.  :-)
// TODO WGJA WIP:  * 
// TODO WGJA WIP:  * C language equivalents written by Theodore Ts'o, 9/26/92
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: extern __inline__ int set_bit(int nr,int * addr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int	mask, retval;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	addr += nr >> 5;
// TODO WGJA WIP: 	mask = 1 << (nr & 0x1f);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	retval = (mask & *addr) != 0;
// TODO WGJA WIP: 	*addr |= mask;
// TODO WGJA WIP: 	sti();
// TODO WGJA WIP: 	return retval;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern __inline__ int clear_bit(int nr, int * addr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int	mask, retval;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	addr += nr >> 5;
// TODO WGJA WIP: 	mask = 1 << (nr & 0x1f);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	retval = (mask & *addr) == 0;
// TODO WGJA WIP: 	*addr &= ~mask;
// TODO WGJA WIP: 	sti();
// TODO WGJA WIP: 	return retval;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern __inline__ int test_bit(int nr, int * addr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int	mask;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	addr += nr >> 5;
// TODO WGJA WIP: 	mask = 1 << (nr & 0x1f);
// TODO WGJA WIP: 	return ((mask & *addr) != 0);
// TODO WGJA WIP: }
#endif	/* i386 */
#endif /* _ASM_BITOPS_H */
