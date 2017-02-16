#ifndef __ASM_SYSTEM_H
#define __ASM_SYSTEM_H

#include <linux/segment.h>

// TODO WGJA WIP: #define move_to_user_mode() \
// TODO WGJA WIP: __asm__ __volatile__ ("movl %%esp,%%eax\n\t" \
// TODO WGJA WIP: 	"pushl %0\n\t" \
// TODO WGJA WIP: 	"pushl %%eax\n\t" \
// TODO WGJA WIP: 	"pushfl\n\t" \
// TODO WGJA WIP: 	"pushl %1\n\t" \
// TODO WGJA WIP: 	"pushl $1f\n\t" \
// TODO WGJA WIP: 	"iret\n" \
// TODO WGJA WIP: 	"1:\tmovl %0,%%eax\n\t" \
// TODO WGJA WIP: 	"mov %%ax,%%ds\n\t" \
// TODO WGJA WIP: 	"mov %%ax,%%es\n\t" \
// TODO WGJA WIP: 	"mov %%ax,%%fs\n\t" \
// TODO WGJA WIP: 	"mov %%ax,%%gs" \
// TODO WGJA WIP: 	: /* no outputs */ :"i" (USER_DS), "i" (USER_CS):"ax")

#define sti() __asm__ __volatile__ ("sti": : :"memory")
#define cli() __asm__ __volatile__ ("cli": : :"memory")
#define nop() __asm__ __volatile__ ("nop")

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Clear and set 'TS' bit respectively
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define clts() __asm__ __volatile__ ("clts")
// TODO WGJA WIP: #define stts() \
// TODO WGJA WIP: __asm__ __volatile__ ( \
// TODO WGJA WIP: 	"movl %%cr0,%%eax\n\t" \
// TODO WGJA WIP: 	"orl $8,%%eax\n\t" \
// TODO WGJA WIP: 	"movl %%eax,%%cr0" \
// TODO WGJA WIP: 	: /* no outputs */ \
// TODO WGJA WIP: 	: /* no inputs */ \
// TODO WGJA WIP: 	:"ax")
// TODO WGJA WIP: 
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline int tas(char * m)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	char res;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	__asm__("xchgb %0,%1":"=q" (res),"=m" (*m):"0" (0x1));
// TODO WGJA WIP: 	return res;
// TODO WGJA WIP: }

#define save_flags(x) \
__asm__ __volatile__("pushfl ; popl %0":"=r" (x): /* no input */ :"memory")

#define restore_flags(x) \
__asm__ __volatile__("pushl %0 ; popfl": /* no output */ :"r" (x):"memory")

// TODO WGJA WIP: #define iret() __asm__ __volatile__ ("iret": : :"memory")

#define _set_gate(gate_addr,type,dpl,addr) \
do { \
  int __d0, __d1; \
  __asm__ __volatile__ ("movw %%dx,%%ax\n\t" \
	"movw %4,%%dx\n\t" \
	"movl %%eax,%0\n\t" \
	"movl %%edx,%1" \
	:"=m" (*((long *) (gate_addr))), \
	 "=m" (*(1+(long *) (gate_addr))), "=&a" (__d0), "=&d" (__d1) \
	:"i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	 "3" ((char *) (addr)),"2" (KERNEL_CS << 16)); \
} while (0)

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define set_call_gate(a,addr) \
	_set_gate(a,12,3,addr)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define _set_seg_desc(gate_addr,type,dpl,base,limit) {\
// TODO WGJA WIP: 	*((gate_addr)+1) = ((base) & 0xff000000) | \
// TODO WGJA WIP: 		(((base) & 0x00ff0000)>>16) | \
// TODO WGJA WIP: 		((limit) & 0xf0000) | \
// TODO WGJA WIP: 		((dpl)<<13) | \
// TODO WGJA WIP: 		(0x00408000) | \
// TODO WGJA WIP: 		((type)<<8); \
// TODO WGJA WIP: 	*(gate_addr) = (((base) & 0x0000ffff)<<16) | \
// TODO WGJA WIP: 		((limit) & 0x0ffff); }
// TODO WGJA WIP: 
// TODO WGJA WIP: #define _set_tssldt_desc(n,addr,limit,type) \
// TODO WGJA WIP: __asm__ __volatile__ ("movw $" #limit ",%1\n\t" \
// TODO WGJA WIP: 	"movw %%ax,%2\n\t" \
// TODO WGJA WIP: 	"rorl $16,%%eax\n\t" \
// TODO WGJA WIP: 	"movb %%al,%3\n\t" \
// TODO WGJA WIP: 	"movb $" type ",%4\n\t" \
// TODO WGJA WIP: 	"movb $0x00,%5\n\t" \
// TODO WGJA WIP: 	"movb %%ah,%6\n\t" \
// TODO WGJA WIP: 	"rorl $16,%%eax" \
// TODO WGJA WIP: 	: /* no output */ \
// TODO WGJA WIP: 	:"a" (addr+0xc0000000), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
// TODO WGJA WIP: 	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
// TODO WGJA WIP: 	)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),235,"0x89")
// TODO WGJA WIP: #define set_ldt_desc(n,addr,size) \
// TODO WGJA WIP: 	_set_tssldt_desc(((char *) (n)),((int)(addr)),((size << 3) - 1),"0x82")
// TODO WGJA WIP: 

#endif
