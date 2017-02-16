/*
 *  linux/kernel/sys_call.S
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * sys_call.S  contains the system-call and fault low-level handling routines.
 * This also contains the timer-interrupt handler, as well as all interrupts
 * and faults that can result in a task-switch.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call.
 *
 * I changed all the .align's to 4 (16 byte alignment), as that's faster
 * on a 486.
 *
 * Stack layout in 'ret_from_system_call':
 * 	ptrace needs to have all regs on the stack.
 *	if the order here is changed, it needs to be 
 *	updated in fork.c:copy_process, signal.c:do_signal,
 *	ptrace.c and ptrace.h
 *
 *	 0(%esp) - %ebx
 *	 4(%esp) - %ecx
 *	 8(%esp) - %edx
 *       C(%esp) - %esi
 *	10(%esp) - %edi
 *	14(%esp) - %ebp
 *	18(%esp) - %eax
 *	1C(%esp) - %ds
 *	20(%esp) - %es
 *      24(%esp) - %fs
 *	28(%esp) - %gs
 *	2C(%esp) - orig_eax
 *	30(%esp) - %eip
 *	34(%esp) - %cs
 *	38(%esp) - %eflags
 *	3C(%esp) - %oldesp
 *	40(%esp) - %oldss
 */

#include <linux/segment.h>

EBX		= 0x00
ECX		= 0x04
EDX		= 0x08
ESI		= 0x0C
EDI		= 0x10
EBP		= 0x14
EAX		= 0x18
DS		= 0x1C
ES		= 0x20
FS		= 0x24
GS		= 0x28
ORIG_EAX	= 0x2C
EIP		= 0x30
CS		= 0x34
EFLAGS		= 0x38
OLDESP		= 0x3C
OLDSS		= 0x40

CF_MASK		= 0x00000001
IF_MASK		= 0x00000200
NT_MASK		= 0x00004000
VM_MASK		= 0x00020000

/*
 * these are offsets into the task-struct.
 */
state		=  0
counter		=  4
priority	=  8
signal		= 12
blocked		= 16
flags		= 20
errno		= 24

ENOSYS = 38

.globl _system_call,lcall7
.globl _device_not_available, _coprocessor_error
.globl _divide_error,_debug,_nmi,_int3,_overflow,_bounds,_invalid_op
.globl _double_fault,_coprocessor_segment_overrun
.globl _invalid_TSS,_segment_not_present,_stack_segment
.globl _general_protection,_reserved
.globl _alignment_check,_page_fault
.globl ret_from_sys_call

#define SAVE_ALL \
	cld; \
	push %gs; \
	push %fs; \
	push %es; \
	push %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $(KERNEL_DS),%edx; \
	mov %dx,%ds; \
	mov %dx,%es; \
	movl $(USER_DS),%edx; \
	mov %dx,%fs

#define RESTORE_ALL \
	popl %ebx; \
	popl %ecx; \
	popl %edx; \
	popl %esi; \
	popl %edi; \
	popl %ebp; \
	popl %eax; \
	pop %ds; \
	pop %es; \
	pop %fs; \
	pop %gs; \
	addl $4,%esp; \
	iret

.align 4
lcall7:
	pushfl			# We get a different stack layout with call gates,
	pushl %eax		# which has to be cleaned up later..
	SAVE_ALL
	movl EIP(%esp),%eax	# due to call gates, this is eflags, not eip..
	movl CS(%esp),%edx	# this is eip..
	movl EFLAGS(%esp),%ecx	# and this is cs..
	movl %eax,EFLAGS(%esp)	#
	movl %edx,EIP(%esp)	# Now we move them to their "normal" places
	movl %ecx,CS(%esp)	#
	movl %esp,%eax
	pushl %eax
	call iABI_emulate
	popl %eax
	jmp ret_from_sys_call

# TODO WGJA: finish sys_call.S

# TODO WGJA WIP: .align 4
# TODO WGJA WIP: handle_bottom_half:
# TODO WGJA WIP: 	pushfl
# TODO WGJA WIP: 	incl _intr_count
# TODO WGJA WIP: 	sti
# TODO WGJA WIP: 	call _do_bottom_half
# TODO WGJA WIP: 	popfl
# TODO WGJA WIP: 	decl _intr_count
# TODO WGJA WIP: 	jmp 9f
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: reschedule:
# TODO WGJA WIP: 	pushl $ret_from_sys_call
# TODO WGJA WIP: 	jmp _schedule
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _system_call:
# TODO WGJA WIP: 	pushl %eax			# save orig_eax
# TODO WGJA WIP: 	SAVE_ALL
# TODO WGJA WIP: 	movl $-ENOSYS,EAX(%esp)
# TODO WGJA WIP: 	cmpl NR_syscalls,%eax
# TODO WGJA WIP: 	jae ret_from_sys_call
# TODO WGJA WIP: 	movl _current,%ebx
# TODO WGJA WIP: 	andl $~CF_MASK,EFLAGS(%esp)	# clear carry - assume no errors
# TODO WGJA WIP: 	movl $0,errno(%ebx)
# TODO WGJA WIP: 	testb $0x20,flags(%ebx)		# PF_TRACESYS
# TODO WGJA WIP: 	jne 1f
# TODO WGJA WIP: 	call _sys_call_table(,%eax,4)
# TODO WGJA WIP: 	movl %eax,EAX(%esp)		# save the return value
# TODO WGJA WIP: 	movl errno(%ebx),%edx
# TODO WGJA WIP: 	negl %edx
# TODO WGJA WIP: 	je ret_from_sys_call
# TODO WGJA WIP: 	movl %edx,EAX(%esp)
# TODO WGJA WIP: 	orl $(CF_MASK),EFLAGS(%esp)	# set carry to indicate error
# TODO WGJA WIP: 	jmp ret_from_sys_call
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: 1:	call _syscall_trace
# TODO WGJA WIP: 	movl ORIG_EAX(%esp),%eax
# TODO WGJA WIP: 	call _sys_call_table(,%eax,4)
# TODO WGJA WIP: 	movl %eax,EAX(%esp)		# save the return value
# TODO WGJA WIP: 	movl _current,%eax
# TODO WGJA WIP: 	movl errno(%eax),%edx
# TODO WGJA WIP: 	negl %edx
# TODO WGJA WIP: 	je 1f
# TODO WGJA WIP: 	movl %edx,EAX(%esp)
# TODO WGJA WIP: 	orl $(CF_MASK),EFLAGS(%esp)	# set carry to indicate error
# TODO WGJA WIP: 1:	call _syscall_trace

# TODO WGJA: finish sys_call.S
ret_from_sys_call_hang_msg:
	.asciz "ret_from_sys_call called. Hanging.\n"

.align 4,0x90
ret_from_sys_call:
	pushl $ret_from_sys_call_hang_msg
	call printk
ret_from_sys_call_hang:
	jmp ret_from_sys_call_hang

# TODO WGJA WIP: 	.align 4,0x90
# TODO WGJA WIP: ret_from_sys_call:
# TODO WGJA WIP: 	cmpl $0,_intr_count
# TODO WGJA WIP: 	jne 2f
# TODO WGJA WIP: 	movl _bh_mask,%eax
# TODO WGJA WIP: 	andl _bh_active,%eax
# TODO WGJA WIP: 	jne handle_bottom_half
# TODO WGJA WIP: 9:	movl EFLAGS(%esp),%eax		# check VM86 flag: CS/SS are
# TODO WGJA WIP: 	testl $(VM_MASK),%eax		# different then
# TODO WGJA WIP: 	jne 1f
# TODO WGJA WIP: 	cmpw $(KERNEL_CS),CS(%esp)	# was old code segment supervisor ?
# TODO WGJA WIP: 	je 2f
# TODO WGJA WIP: 1:	sti
# TODO WGJA WIP: 	orl $(IF_MASK),%eax		# these just try to make sure
# TODO WGJA WIP: 	andl $~NT_MASK,%eax		# the program doesn't do anything
# TODO WGJA WIP: 	movl %eax,EFLAGS(%esp)		# stupid
# TODO WGJA WIP: 	cmpl $0,_need_resched
# TODO WGJA WIP: 	jne reschedule
# TODO WGJA WIP: 	movl _current,%eax
# TODO WGJA WIP: 	cmpl _task,%eax			# task[0] cannot have signals
# TODO WGJA WIP: 	je 2f
# TODO WGJA WIP: 	cmpl $0,state(%eax)		# state
# TODO WGJA WIP: 	jne reschedule
# TODO WGJA WIP: 	cmpl $0,counter(%eax)		# counter
# TODO WGJA WIP: 	je reschedule
# TODO WGJA WIP: 	movl blocked(%eax),%ecx
# TODO WGJA WIP: 	movl %ecx,%ebx			# save blocked in %ebx for signal handling
# TODO WGJA WIP: 	notl %ecx
# TODO WGJA WIP: 	andl signal(%eax),%ecx
# TODO WGJA WIP: 	jne signal_return
# TODO WGJA WIP: 2:	RESTORE_ALL
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: signal_return:
# TODO WGJA WIP: 	movl %esp,%ecx
# TODO WGJA WIP: 	pushl %ecx
# TODO WGJA WIP: 	testl $(VM_MASK),EFLAGS(%ecx)
# TODO WGJA WIP: 	jne v86_signal_return
# TODO WGJA WIP: 	pushl %ebx
# TODO WGJA WIP: 	call do_signal
# TODO WGJA WIP: 	popl %ebx
# TODO WGJA WIP: 	popl %ebx
# TODO WGJA WIP: 	RESTORE_ALL
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: v86_signal_return:
# TODO WGJA WIP: 	call _save_v86_state
# TODO WGJA WIP: 	movl %eax,%esp
# TODO WGJA WIP: 	pushl %eax
# TODO WGJA WIP: 	pushl %ebx
# TODO WGJA WIP: 	call _do_signal
# TODO WGJA WIP: 	popl %ebx
# TODO WGJA WIP: 	popl %ebx
# TODO WGJA WIP: 	RESTORE_ALL
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _divide_error:
# TODO WGJA WIP: 	pushl $0		# no error code
# TODO WGJA WIP: 	pushl $_do_divide_error
# TODO WGJA WIP: .align 4,0x90
# TODO WGJA WIP: error_code:
# TODO WGJA WIP: 	push %fs
# TODO WGJA WIP: 	push %es
# TODO WGJA WIP: 	push %ds
# TODO WGJA WIP: 	pushl %eax
# TODO WGJA WIP: 	pushl %ebp
# TODO WGJA WIP: 	pushl %edi
# TODO WGJA WIP: 	pushl %esi
# TODO WGJA WIP: 	pushl %edx
# TODO WGJA WIP: 	pushl %ecx
# TODO WGJA WIP: 	pushl %ebx
# TODO WGJA WIP: 	cld
# TODO WGJA WIP: 	movl $-1, %eax
# TODO WGJA WIP: 	xchgl %eax, ORIG_EAX(%esp)	# orig_eax (get the error code. )
# TODO WGJA WIP: 	xorl %ebx,%ebx			# zero ebx
# TODO WGJA WIP: 	mov %gs,%bx			# get the lower order bits of gs
# TODO WGJA WIP: 	xchgl %ebx, GS(%esp)		# get the address and save gs.
# TODO WGJA WIP: 	pushl %eax			# push the error code
# TODO WGJA WIP: 	lea 4(%esp),%edx
# TODO WGJA WIP: 	pushl %edx
# TODO WGJA WIP: 	movl $(KERNEL_DS),%edx
# TODO WGJA WIP: 	mov %dx,%ds
# TODO WGJA WIP: 	mov %dx,%es
# TODO WGJA WIP: 	movl $(USER_DS),%edx
# TODO WGJA WIP: 	mov %dx,%fs
# TODO WGJA WIP: 	call *%ebx
# TODO WGJA WIP: 	addl $8,%esp
# TODO WGJA WIP: 	jmp ret_from_sys_call
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _coprocessor_error:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_coprocessor_error
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _device_not_available:
# TODO WGJA WIP: 	pushl $-1		# mark this as an int
# TODO WGJA WIP: 	SAVE_ALL
# TODO WGJA WIP: 	pushl $ret_from_sys_call
# TODO WGJA WIP: 	movl %cr0,%eax
# TODO WGJA WIP: 	testl $0x4,%eax			# EM (math emulation bit)
# TODO WGJA WIP: 	je _math_state_restore
# TODO WGJA WIP: 	pushl $0		# temporary storage for ORIG_EIP
# TODO WGJA WIP: 	call _math_emulate
# TODO WGJA WIP: 	addl $4,%esp
# TODO WGJA WIP: 	ret
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _debug:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_debug
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _nmi:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_nmi
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _int3:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_int3
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _overflow:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_overflow
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _bounds:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_bounds
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _invalid_op:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_invalid_op
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _coprocessor_segment_overrun:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_coprocessor_segment_overrun
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _reserved:
# TODO WGJA WIP: 	pushl $0
# TODO WGJA WIP: 	pushl $_do_reserved
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _double_fault:
# TODO WGJA WIP: 	pushl $_do_double_fault
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _invalid_TSS:
# TODO WGJA WIP: 	pushl $_do_invalid_TSS
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _segment_not_present:
# TODO WGJA WIP: 	pushl $_do_segment_not_present
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _stack_segment:
# TODO WGJA WIP: 	pushl $_do_stack_segment
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _general_protection:
# TODO WGJA WIP: 	pushl $_do_general_protection
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _alignment_check:
# TODO WGJA WIP: 	pushl $_do_alignment_check
# TODO WGJA WIP: 	jmp error_code
# TODO WGJA WIP: 
# TODO WGJA WIP: .align 4
# TODO WGJA WIP: _page_fault:
# TODO WGJA WIP: 	pushl $_do_page_fault
# TODO WGJA WIP: 	jmp error_code
