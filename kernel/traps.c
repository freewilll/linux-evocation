/*
 *  linux/kernel/traps.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * 'Traps.c' handles hardware traps and faults after we have saved some
 * state in 'asm.s'. Currently mostly a debugging-aid, will be extended
 * to mainly kill the offending process (probably by giving it a signal,
 * but possibly by killing it outright if necessary).
 */
#include <linux/head.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/segment.h>
#include <linux/ptrace.h>

#include <asm/system.h>
#include <asm/segment.h>
#include <asm/io.h>

#define get_seg_byte(seg,addr) ({ \
register char __res; \
__asm__("push %%fs;mov %%ax,%%fs;movb %%fs:%2,%%al;pop %%fs" \
	:"=a" (__res):"0" (seg),"m" (*(addr))); \
__res;})

// TODO WGJA WIP: #define get_seg_long(seg,addr) ({ \
// TODO WGJA WIP: register unsigned long __res; \
// TODO WGJA WIP: __asm__("push %%fs;mov %%ax,%%fs;movl %%fs:%2,%%eax;pop %%fs" \
// TODO WGJA WIP: 	:"=a" (__res):"0" (seg),"m" (*(addr))); \
// TODO WGJA WIP: __res;})
// TODO WGJA WIP: 
// TODO WGJA WIP: #define _fs() ({ \
// TODO WGJA WIP: register unsigned short __res; \
// TODO WGJA WIP: __asm__("mov %%fs,%%ax":"=a" (__res):); \
// TODO WGJA WIP: __res;})
// TODO WGJA WIP: 
// TODO WGJA WIP: void page_exception(void);

extern "C" void divide_error(void);
extern "C" void debug(void);
extern "C" void nmi(void);
extern "C" void int3(void);
extern "C" void overflow(void);
extern "C" void bounds(void);
extern "C" void invalid_op(void);
extern "C" void device_not_available(void);
extern "C" void double_fault(void);
extern "C" void coprocessor_segment_overrun(void);
extern "C" void invalid_TSS(void);
extern "C" void segment_not_present(void);
extern "C" void stack_segment(void);
extern "C" void general_protection(void);
extern "C" void page_fault(void);
extern "C" void coprocessor_error(void);
extern "C" void reserved(void);
extern "C" void alignment_check(void);

/*static*/ void die_if_kernel(char * str, struct pt_regs * regs, long err)
{
	int i;

	if ((regs->eflags & VM_MASK) || ((0xffff & regs->cs) == USER_CS))
		return;
	printk("%s: %04x\n", str, err & 0xffff);
	printk("EIP:    %04x:%p\nEFLAGS: %p\n", 0xffff & regs->cs,regs->eip,regs->eflags);
	printk("eax: %08x   ebx: %08x   ecx: %08x   edx: %08x\n",
		regs->eax, regs->ebx, regs->ecx, regs->edx);
	printk("esi: %08x   edi: %08x   ebp: %08x\n",
		regs->esi, regs->edi, regs->ebp);
	printk("ds: %04x   es: %04x   fs: %04x   gs: %04x\n",
		regs->ds, regs->es, regs->fs, regs->gs);
	store_TR(i);
	printk("Pid: %d, process nr: %d\n", current->pid, 0xffff & i);
	for(i=0;i<10;i++)
		printk("%02x ",0xff & get_seg_byte(regs->cs,(i+(char *)regs->eip)));
	printk("\n");
	// do_exit(SIGSEGV);  // TODO WGJA die_if_kernel
	cli();

}

// TODO WGJA WIP: extern "C" void do_double_fault(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("double fault",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_general_protection(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("general protection",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_alignment_check(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("alignment check",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_divide_error(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGFPE, current, 1);
// TODO WGJA WIP: 	die_if_kernel("divide error",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_int3(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (current->flags & PF_PTRACED)
// TODO WGJA WIP: 		current->blocked &= ~(1 << (SIGTRAP-1));
// TODO WGJA WIP: 	send_sig(SIGTRAP, current, 1);
// TODO WGJA WIP: 	die_if_kernel("int3",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_nmi(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	printk("Uhhuh. NMI received. Dazed and confused, but trying to continue\n");
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_debug(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (current->flags & PF_PTRACED)
// TODO WGJA WIP: 		current->blocked &= ~(1 << (SIGTRAP-1));
// TODO WGJA WIP: 	send_sig(SIGTRAP, current, 1);
// TODO WGJA WIP: 	die_if_kernel("debug",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_overflow(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("overflow",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_bounds(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("bounds",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_invalid_op(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGILL, current, 1);
// TODO WGJA WIP: 	die_if_kernel("invalid operand",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_device_not_available(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("device not available",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_coprocessor_segment_overrun(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGFPE, last_task_used_math, 1);
// TODO WGJA WIP: 	die_if_kernel("coprocessor segment overrun",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_invalid_TSS(struct pt_regs * regs,long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("invalid TSS",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_segment_not_present(struct pt_regs * regs,long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("segment not present",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_stack_segment(struct pt_regs * regs,long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("stack segment",regs,error_code);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Allow the process which triggered the interrupt to recover the error
// TODO WGJA WIP:  * condition.
// TODO WGJA WIP:  *  - the status word is saved in the cs selector.
// TODO WGJA WIP:  *  - the tag word is saved in the operand selector.
// TODO WGJA WIP:  *  - the status word is then cleared and the tags all set to Empty.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * This will give sufficient information for complete recovery provided that
// TODO WGJA WIP:  * the affected process knows or can deduce the code and data segments
// TODO WGJA WIP:  * which were in force when the exception condition arose.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Note that we play around with the 'TS' bit to hopefully get
// TODO WGJA WIP:  * the correct behaviour even in the presense of the asynchronous
// TODO WGJA WIP:  * IRQ13 behaviour
// TODO WGJA WIP:  */
void math_error(void)
{
	// TODO WGJA enable math_error
	printk("Math error. Hanging.");
	for (;;);
// TODO WGJA WIP: 	struct i387_hard_struct * env;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	clts();
// TODO WGJA WIP: 	if (!last_task_used_math) {
// TODO WGJA WIP: 		__asm__("fnclex");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	env = &last_task_used_math->tss.i387.hard;
// TODO WGJA WIP: 	send_sig(SIGFPE, last_task_used_math, 1);
// TODO WGJA WIP: 	__asm__ __volatile__("fnsave %0":"=m" (*env));
// TODO WGJA WIP: 	last_task_used_math = NULL;
// TODO WGJA WIP: 	stts();
// TODO WGJA WIP: 	env->fcs = (env->swd & 0x0000ffff) | (env->fcs & 0xffff0000);
// TODO WGJA WIP: 	env->fos = env->twd;
// TODO WGJA WIP: 	env->swd &= 0xffff0000;
// TODO WGJA WIP: 	env->twd = 0xffffffff;
}

// TODO WGJA WIP: extern "C" void do_coprocessor_error(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	ignore_irq13 = 1;
// TODO WGJA WIP: 	math_error();
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void do_reserved(struct pt_regs * regs, long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 	die_if_kernel("reserved (15,17-47) error",regs,error_code);
// TODO WGJA WIP: }

// TODO WGJA enable traps
#define UNHT(n) void unhandled_trap_##n(struct pt_regs * regs, long error_code) { \
	char *vidmem = (char *)0xb8000; vidmem[79 * 2] = 'T'; vidmem[79 * 2 + 1] = 0x40; for (;;); \
	}
#define STG(n) set_trap_gate(n,&unhandled_trap_##n);
UNHT(0);
UNHT(1);
UNHT(2);
UNHT(3);
UNHT(4);
UNHT(5);
UNHT(6);
UNHT(7);
UNHT(8);
UNHT(9);
UNHT(10);
UNHT(11);
UNHT(12);
UNHT(13);
UNHT(14);
UNHT(15);
UNHT(16);
UNHT(17);

void unhandled_trap() { 
	printk("Unhandled trap. Giving up.\n"); 
	for (;;); 
}

void trap_init(void)
{
	STG(0);
	STG(1);
	STG(2);
	STG(3);
	STG(4);
	STG(5);
	STG(6);
	STG(7);
	STG(8);
	STG(9);
	STG(10);
	STG(11);
	STG(12);
	STG(13);
	STG(14);
	STG(15);
	STG(16);
	STG(17);

	int i;

	// TODO WGJA enable traps
	for (i=18;i<48;i++)
		set_trap_gate(i,&unhandled_trap);

	// set_trap_gate(0,&divide_error);
	// set_trap_gate(1,&debug);
	// set_trap_gate(2,&nmi);
	// set_system_gate(3,&int3);	/* int3-5 can be called from all */
	// set_system_gate(4,&overflow);
	// set_system_gate(5,&bounds);
	// set_trap_gate(6,&invalid_op);
	// set_trap_gate(7,&device_not_available);
	// set_trap_gate(8,&double_fault);
	// set_trap_gate(9,&coprocessor_segment_overrun);
	// set_trap_gate(10,&invalid_TSS);
	// set_trap_gate(11,&segment_not_present);
	// set_trap_gate(12,&stack_segment);
	// set_trap_gate(13,&general_protection);
	set_trap_gate(14,&page_fault);
	// set_trap_gate(15,&reserved);
	// set_trap_gate(16,&coprocessor_error);
	// set_trap_gate(17,&alignment_check);
	// for (i=18;i<48;i++)
	// 	set_trap_gate(i,&reserved);
}
