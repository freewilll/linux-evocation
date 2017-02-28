/*
 *  linux/kernel/sys.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/errno.h>
#include <linux/sched.h>
// TODO WGJA WIP: #include <linux/tty.h>
#include <linux/kernel.h>
#include <linux/config.h>
// TODO WGJA WIP: #include <linux/times.h>
#include <linux/utsname.h>
#include <linux/param.h>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/stat.h>

#include <asm/segment.h>
#include <asm/io.h>

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * this indicates wether you can reboot with ctrl-alt-del: the default is yes
// TODO WGJA WIP:  */
// TODO WGJA WIP: static int C_A_D = 1;
// TODO WGJA WIP: 
// TODO WGJA WIP: /* 
// TODO WGJA WIP:  * The timezone where the local system is located.  Used as a default by some
// TODO WGJA WIP:  * programs who obtain this value by using gettimeofday.
// TODO WGJA WIP:  */
// TODO WGJA WIP: struct timezone sys_tz = { 0, 0};
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int session_of_pgrp(int pgrp);
// TODO WGJA WIP: extern void adjust_clock(void);
// TODO WGJA WIP: 
// TODO WGJA WIP: #define	PZERO	15
// TODO WGJA WIP: 
// TODO WGJA WIP: static int proc_sel(struct task_struct *p, int which, int who)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	switch (which) {
// TODO WGJA WIP: 		case PRIO_PROCESS:
// TODO WGJA WIP: 			if (!who && p == current)
// TODO WGJA WIP: 				return 1;
// TODO WGJA WIP: 			return(p->pid == who);
// TODO WGJA WIP: 		case PRIO_PGRP:
// TODO WGJA WIP: 			if (!who)
// TODO WGJA WIP: 				who = current->pgrp;
// TODO WGJA WIP: 			return(p->pgrp == who);
// TODO WGJA WIP: 		case PRIO_USER:
// TODO WGJA WIP: 			if (!who)
// TODO WGJA WIP: 				who = current->uid;
// TODO WGJA WIP: 			return(p->uid == who);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_setpriority(int which, int who, int niceval)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct task_struct **p;
// TODO WGJA WIP: 	int error = ESRCH;
// TODO WGJA WIP: 	int priority;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (which > 2 || which < 0)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if ((priority = PZERO - niceval) <= 0)
// TODO WGJA WIP: 		priority = 1;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for(p = &LAST_TASK; p > &FIRST_TASK; --p) {
// TODO WGJA WIP: 		if (!*p || !proc_sel(*p, which, who))
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if ((*p)->uid != current->euid &&
// TODO WGJA WIP: 			(*p)->uid != current->uid && !suser()) {
// TODO WGJA WIP: 			error = EPERM;
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (error == ESRCH)
// TODO WGJA WIP: 			error = 0;
// TODO WGJA WIP: 		if (priority > (*p)->priority && !suser())
// TODO WGJA WIP: 			error = EACCES;
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			(*p)->priority = priority;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return -error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getpriority(int which, int who)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct task_struct **p;
// TODO WGJA WIP: 	int max_prio = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (which > 2 || which < 0)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for(p = &LAST_TASK; p > &FIRST_TASK; --p) {
// TODO WGJA WIP: 		if (!*p || !proc_sel(*p, which, who))
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if ((*p)->priority > max_prio)
// TODO WGJA WIP: 			max_prio = (*p)->priority;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return(max_prio ? max_prio : -ESRCH);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_profil(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_ftime(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_break(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_stty(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_gtty(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_prof(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" unsigned long save_v86_state(struct vm86_regs * regs)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long stack;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!current->vm86_info) {
// TODO WGJA WIP: 		printk("no vm86_info: BAD\n");
// TODO WGJA WIP: 		do_exit(SIGSEGV);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	memcpy_tofs(&(current->vm86_info->regs),regs,sizeof(*regs));
// TODO WGJA WIP: 	put_fs_long(current->screen_bitmap,&(current->vm86_info->screen_bitmap));
// TODO WGJA WIP: 	stack = current->tss.esp0;
// TODO WGJA WIP: 	current->tss.esp0 = current->saved_kernel_stack;
// TODO WGJA WIP: 	current->saved_kernel_stack = 0;
// TODO WGJA WIP: 	return stack;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static void mark_screen_rdonly(struct task_struct * tsk)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long tmp;
// TODO WGJA WIP: 	unsigned long *pg_table;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if ((tmp = tsk->tss.cr3) != 0) {
// TODO WGJA WIP: 		tmp = *(unsigned long *) tmp;
// TODO WGJA WIP: 		if (tmp & PAGE_PRESENT) {
// TODO WGJA WIP: 			tmp &= PAGE_MASK;
// TODO WGJA WIP: 			pg_table = (0xA0000 >> PAGE_SHIFT) + (unsigned long *) tmp;
// TODO WGJA WIP: 			tmp = 32;
// TODO WGJA WIP: 			while (tmp--) {
// TODO WGJA WIP: 				if (PAGE_PRESENT & *pg_table)
// TODO WGJA WIP: 					*pg_table &= ~PAGE_RW;
// TODO WGJA WIP: 				pg_table++;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_vm86(struct vm86_struct * v86)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct vm86_struct info;
// TODO WGJA WIP: 	struct pt_regs * pt_regs = (struct pt_regs *) &v86;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (current->saved_kernel_stack)
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	memcpy_fromfs(&info,v86,sizeof(info));
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * make sure the vm86() system call doesn't try to do anything silly
// TODO WGJA WIP:  */
// TODO WGJA WIP: 	info.regs.__null_ds = 0;
// TODO WGJA WIP: 	info.regs.__null_es = 0;
// TODO WGJA WIP: 	info.regs.__null_fs = 0;
// TODO WGJA WIP: 	info.regs.__null_gs = 0;
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * The eflags register is also special: we cannot trust that the user
// TODO WGJA WIP:  * has set it up safely, so this makes sure interrupt etc flags are
// TODO WGJA WIP:  * inherited from protected mode.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 	info.regs.eflags &= 0x00000dd5;
// TODO WGJA WIP: 	info.regs.eflags |= ~0x00000dd5 & pt_regs->eflags;
// TODO WGJA WIP: 	info.regs.eflags |= VM_MASK;
// TODO WGJA WIP: 	current->saved_kernel_stack = current->tss.esp0;
// TODO WGJA WIP: 	current->tss.esp0 = (unsigned long) pt_regs;
// TODO WGJA WIP: 	current->vm86_info = v86;
// TODO WGJA WIP: 	current->screen_bitmap = info.screen_bitmap;
// TODO WGJA WIP: 	if (info.flags & VM86_SCREEN_BITMAP)
// TODO WGJA WIP: 		mark_screen_rdonly(current);
// TODO WGJA WIP: 	__asm__ __volatile__("movl %0,%%esp\n\t"
// TODO WGJA WIP: 		"pushl $ret_from_sys_call\n\t"
// TODO WGJA WIP: 		"ret"
// TODO WGJA WIP: 		: /* no outputs */
// TODO WGJA WIP: 		:"g" ((long) &(info.regs)),"a" (info.regs.eax));
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void hard_reset_now(void);
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Reboot system call: for obvious reasons only root may call it,
// TODO WGJA WIP:  * and even root needs to set up some magic numbers in the registers
// TODO WGJA WIP:  * so that some mistake won't make this reboot the whole machine.
// TODO WGJA WIP:  * You can also set the meaning of the ctrl-alt-del-key here.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * reboot doesn't sync: do that yourself before calling this.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_reboot(int magic, int magic_too, int flag)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if (magic != 0xfee1dead || magic_too != 672274793)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	if (flag == 0x01234567)
// TODO WGJA WIP: 		hard_reset_now();
// TODO WGJA WIP: 	else if (flag == 0x89ABCDEF)
// TODO WGJA WIP: 		C_A_D = 1;
// TODO WGJA WIP: 	else if (!flag)
// TODO WGJA WIP: 		C_A_D = 0;
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	return (0);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This function gets called by ctrl-alt-del - ie the keyboard interrupt.
// TODO WGJA WIP:  * As it's called within an interrupt, it may NOT sync: the only choice
// TODO WGJA WIP:  * is wether to reboot at once, or just ignore the ctrl-alt-del.
// TODO WGJA WIP:  */
// TODO WGJA WIP: void ctrl_alt_del(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (C_A_D)
// TODO WGJA WIP: 		hard_reset_now();
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		send_sig(SIGINT,task[1],1);
// TODO WGJA WIP: }
	// TODO WGJA WIP: 
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This is done BSD-style, with no consideration of the saved gid, except
// TODO WGJA WIP:  * that if you set the effective gid, it sets the saved gid too.  This 
// TODO WGJA WIP:  * makes it possible for a setgid program to completely drop its privileges,
// TODO WGJA WIP:  * which is often a useful assertion to make when you are doing a security
// TODO WGJA WIP:  * audit over a program.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * The general idea is that a program which uses just setregid() will be
// TODO WGJA WIP:  * 100% compatible with BSD.  A program which uses just setgid() will be
// TODO WGJA WIP:  * 100% compatible with POSIX w/ Saved ID's. 
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_setregid(gid_t rgid, gid_t egid)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int old_rgid = current->gid;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (rgid != (gid_t) -1) {
// TODO WGJA WIP: 		if ((current->egid==rgid) ||
// TODO WGJA WIP: 		    (old_rgid == rgid) || 
// TODO WGJA WIP: 		    suser())
// TODO WGJA WIP: 			current->gid = rgid;
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			return(-EPERM);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (egid != (gid_t) -1) {
// TODO WGJA WIP: 		if ((old_rgid == egid) ||
// TODO WGJA WIP: 		    (current->egid == egid) ||
// TODO WGJA WIP: 		    suser()) {
// TODO WGJA WIP: 			current->egid = egid;
// TODO WGJA WIP: 			current->sgid = egid;
// TODO WGJA WIP: 		} else {
// TODO WGJA WIP: 			current->gid = old_rgid;
// TODO WGJA WIP: 			return(-EPERM);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * setgid() is implemeneted like SysV w/ SAVED_IDS 
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_setgid(gid_t gid)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (suser())
// TODO WGJA WIP: 		current->gid = current->egid = current->sgid = gid;
// TODO WGJA WIP: 	else if ((gid == current->gid) || (gid == current->sgid))
// TODO WGJA WIP: 		current->egid = gid;
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_acct(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_phys(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_lock(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_mpx(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_ulimit(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_old_syscall(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_time(long * tloc)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i, error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	i = CURRENT_TIME;
// TODO WGJA WIP: 	if (tloc) {
// TODO WGJA WIP: 		error = verify_area(VERIFY_WRITE, tloc, 4);
// TODO WGJA WIP: 		if (error)
// TODO WGJA WIP: 			return error;
// TODO WGJA WIP: 		put_fs_long(i,(unsigned long *)tloc);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return i;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Unprivileged users may change the real user id to the effective uid
// TODO WGJA WIP:  * or vice versa.  (BSD-style)
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * When you set the effective uid, it sets the saved uid too.  This 
// TODO WGJA WIP:  * makes it possible for a setuid program to completely drop its privileges,
// TODO WGJA WIP:  * which is often a useful assertion to make when you are doing a security
// TODO WGJA WIP:  * audit over a program.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * The general idea is that a program which uses just setreuid() will be
// TODO WGJA WIP:  * 100% compatible with BSD.  A program which uses just setuid() will be
// TODO WGJA WIP:  * 100% compatible with POSIX w/ Saved ID's. 
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_setreuid(uid_t ruid, uid_t euid)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int old_ruid = current->uid;
	// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (ruid != (uid_t) -1) {
// TODO WGJA WIP: 		if ((current->euid==ruid) ||
// TODO WGJA WIP: 		    (old_ruid == ruid) ||
// TODO WGJA WIP: 		    suser())
// TODO WGJA WIP: 			current->uid = ruid;
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			return(-EPERM);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (euid != (uid_t) -1) {
// TODO WGJA WIP: 		if ((old_ruid == euid) ||
// TODO WGJA WIP: 		    (current->euid == euid) ||
// TODO WGJA WIP: 		    suser()) {
// TODO WGJA WIP: 			current->euid = euid;
// TODO WGJA WIP: 			current->suid = euid;
// TODO WGJA WIP: 		} else {
// TODO WGJA WIP: 			current->uid = old_ruid;
// TODO WGJA WIP: 			return(-EPERM);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * setuid() is implemeneted like SysV w/ SAVED_IDS 
// TODO WGJA WIP:  * 
// TODO WGJA WIP:  * Note that SAVED_ID's is deficient in that a setuid root program
// TODO WGJA WIP:  * like sendmail, for example, cannot set its uid to be a normal 
// TODO WGJA WIP:  * user and then switch back, because if you're root, setuid() sets
// TODO WGJA WIP:  * the saved uid too.  If you don't like this, blame the bright people
// TODO WGJA WIP:  * in the POSIX commmittee and/or USG.  Note that the BSD-style setreuid()
// TODO WGJA WIP:  * will allow a root program to temporarily drop privileges and be able to
// TODO WGJA WIP:  * regain them by swapping the real and effective uid.  
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_setuid(uid_t uid)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (suser())
// TODO WGJA WIP: 		current->uid = current->euid = current->suid = uid;
// TODO WGJA WIP: 	else if ((uid == current->uid) || (uid == current->suid))
// TODO WGJA WIP: 		current->euid = uid;
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	return(0);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_stime(long * tptr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	startup_time = get_fs_long((unsigned long *)tptr) - jiffies/HZ;
// TODO WGJA WIP: 	jiffies_offset = 0;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_times(struct tms * tbuf)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (tbuf) {
// TODO WGJA WIP: 		int error = verify_area(VERIFY_WRITE,tbuf,sizeof *tbuf);
// TODO WGJA WIP: 		if (error)
// TODO WGJA WIP: 			return error;
// TODO WGJA WIP: 		put_fs_long(current->utime,(unsigned long *)&tbuf->tms_utime);
// TODO WGJA WIP: 		put_fs_long(current->stime,(unsigned long *)&tbuf->tms_stime);
// TODO WGJA WIP: 		put_fs_long(current->cutime,(unsigned long *)&tbuf->tms_cutime);
// TODO WGJA WIP: 		put_fs_long(current->cstime,(unsigned long *)&tbuf->tms_cstime);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return jiffies;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_brk(unsigned long brk)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int freepages;
// TODO WGJA WIP: 	unsigned long rlim;
// TODO WGJA WIP: 	unsigned long newbrk, oldbrk;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (brk < current->end_code)
// TODO WGJA WIP: 		return current->brk;
// TODO WGJA WIP: 	newbrk = PAGE_ALIGN(brk);
// TODO WGJA WIP: 	oldbrk = PAGE_ALIGN(current->brk);
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Always allow shrinking brk
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if (brk <= current->brk) {
// TODO WGJA WIP: 		current->brk = brk;
// TODO WGJA WIP: 		unmap_page_range(newbrk, oldbrk-newbrk);
// TODO WGJA WIP: 		return brk;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Check against rlimit and stack..
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	rlim = current->rlim[RLIMIT_DATA].rlim_cur;
// TODO WGJA WIP: 	if (rlim >= RLIM_INFINITY)
// TODO WGJA WIP: 		rlim = ~0;
// TODO WGJA WIP: 	if (brk - current->end_code > rlim || brk >= current->start_stack - 16384)
// TODO WGJA WIP: 		return current->brk;
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * stupid algorithm to decide if we have enough memory: while
// TODO WGJA WIP: 	 * simple, it hopefully works in most obvious cases.. Easy to
// TODO WGJA WIP: 	 * fool it, but this should catch most mistakes.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	freepages = buffermem >> 12;
// TODO WGJA WIP: 	freepages += nr_free_pages;
// TODO WGJA WIP: 	freepages += nr_swap_pages;
// TODO WGJA WIP: 	freepages -= (high_memory - 0x100000) >> 16;
// TODO WGJA WIP: 	freepages -= (newbrk-oldbrk) >> 12;
// TODO WGJA WIP: 	if (freepages < 0)
// TODO WGJA WIP: 		return current->brk;
// TODO WGJA WIP: #if 0
// TODO WGJA WIP: 	freepages += current->rss;
// TODO WGJA WIP: 	freepages -= oldbrk >> 12;
// TODO WGJA WIP: 	if (freepages < 0)
// TODO WGJA WIP: 		return current->brk;
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Ok, we have probably got enough memory - let it rip.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	current->brk = brk;
// TODO WGJA WIP: 	zeromap_page_range(oldbrk, newbrk-oldbrk, PAGE_COPY);
// TODO WGJA WIP: 	return brk;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This needs some heave checking ...
// TODO WGJA WIP:  * I just haven't get the stomach for it. I also don't fully
// TODO WGJA WIP:  * understand sessions/pgrp etc. Let somebody who does explain it.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * OK, I think I have the protection semantics right.... this is really
// TODO WGJA WIP:  * only important on a multi-user system anyway, to make sure one user
// TODO WGJA WIP:  * can't send a signal to a process owned by another.  -TYT, 12/12/91
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_setpgid(pid_t pid, pid_t pgid)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i; 
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!pid)
// TODO WGJA WIP: 		pid = current->pid;
// TODO WGJA WIP: 	if (!pgid)
// TODO WGJA WIP: 		pgid = current->pid;
// TODO WGJA WIP: 	if (pgid < 0)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	for (i=0 ; i<NR_TASKS ; i++)
// TODO WGJA WIP: 		if (task[i] && (task[i]->pid == pid) &&
// TODO WGJA WIP: 		    ((task[i]->p_pptr == current) || 
// TODO WGJA WIP: 		     (task[i] == current))) {
// TODO WGJA WIP: 			if (task[i]->leader)
// TODO WGJA WIP: 				return -EPERM;
// TODO WGJA WIP: 			if ((task[i]->session != current->session) ||
// TODO WGJA WIP: 			    ((pgid != pid) && 
// TODO WGJA WIP: 			     (session_of_pgrp(pgid) != current->session)))
// TODO WGJA WIP: 				return -EPERM;
// TODO WGJA WIP: 			task[i]->pgrp = pgid;
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	return -ESRCH;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getpgrp(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return current->pgrp;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_setsid(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (current->leader && !suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	current->leader = 1;
// TODO WGJA WIP: 	current->session = current->pgrp = current->pid;
// TODO WGJA WIP: 	current->tty = -1;
// TODO WGJA WIP: 	return current->pgrp;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Supplementary group ID's
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_getgroups(int gidsetsize, gid_t *grouplist)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (gidsetsize) {
// TODO WGJA WIP: 		i = verify_area(VERIFY_WRITE, grouplist, sizeof(gid_t) * gidsetsize);
// TODO WGJA WIP: 		if (i)
// TODO WGJA WIP: 			return i;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	for (i = 0 ; (i < NGROUPS) && (current->groups[i] != NOGROUP) ; i++) {
// TODO WGJA WIP: 		if (!gidsetsize)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (i >= gidsetsize)
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		put_fs_word(current->groups[i], (short *) grouplist);
// TODO WGJA WIP: 		grouplist++;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return(i);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_setgroups(int gidsetsize, gid_t *grouplist)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int	i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if (gidsetsize > NGROUPS)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	for (i = 0; i < gidsetsize; i++, grouplist++) {
// TODO WGJA WIP: 		current->groups[i] = get_fs_word((unsigned short *) grouplist);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (i < NGROUPS)
// TODO WGJA WIP: 		current->groups[i] = NOGROUP;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }

int in_group_p(gid_t grp)
{
	int	i;

	if (grp == current->egid)
		return 1;

	for (i = 0; i < NGROUPS; i++) {
		if (current->groups[i] == NOGROUP)
			break;
		if (current->groups[i] == grp)
			return 1;
	}
	return 0;
}

// TODO WGJA WIP: extern "C" int sys_newuname(struct new_utsname * name)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!name)
// TODO WGJA WIP: 		return -EFAULT;
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE, name, sizeof *name);
// TODO WGJA WIP: 	if (!error)
// TODO WGJA WIP: 		memcpy_tofs(name,&system_utsname,sizeof *name);
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_uname(struct old_utsname * name)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	if (!name)
// TODO WGJA WIP: 		return -EFAULT;
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE, name,sizeof *name);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	memcpy_tofs(&name->sysname,&system_utsname.sysname,
// TODO WGJA WIP: 		sizeof (system_utsname.sysname));
// TODO WGJA WIP: 	memcpy_tofs(&name->nodename,&system_utsname.nodename,
// TODO WGJA WIP: 		sizeof (system_utsname.nodename));
// TODO WGJA WIP: 	memcpy_tofs(&name->release,&system_utsname.release,
// TODO WGJA WIP: 		sizeof (system_utsname.release));
// TODO WGJA WIP: 	memcpy_tofs(&name->version,&system_utsname.version,
// TODO WGJA WIP: 		sizeof (system_utsname.version));
// TODO WGJA WIP: 	memcpy_tofs(&name->machine,&system_utsname.machine,
// TODO WGJA WIP: 		sizeof (system_utsname.machine));
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_olduname(struct oldold_utsname * name)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	if (!name)
// TODO WGJA WIP: 		return -EFAULT;
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE, name,sizeof *name);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	memcpy_tofs(&name->sysname,&system_utsname.sysname,__OLD_UTS_LEN);
// TODO WGJA WIP: 	put_fs_byte(0,name->sysname+__OLD_UTS_LEN);
// TODO WGJA WIP: 	memcpy_tofs(&name->nodename,&system_utsname.nodename,__OLD_UTS_LEN);
// TODO WGJA WIP: 	put_fs_byte(0,name->nodename+__OLD_UTS_LEN);
// TODO WGJA WIP: 	memcpy_tofs(&name->release,&system_utsname.release,__OLD_UTS_LEN);
// TODO WGJA WIP: 	put_fs_byte(0,name->release+__OLD_UTS_LEN);
// TODO WGJA WIP: 	memcpy_tofs(&name->version,&system_utsname.version,__OLD_UTS_LEN);
// TODO WGJA WIP: 	put_fs_byte(0,name->version+__OLD_UTS_LEN);
// TODO WGJA WIP: 	memcpy_tofs(&name->machine,&system_utsname.machine,__OLD_UTS_LEN);
// TODO WGJA WIP: 	put_fs_byte(0,name->machine+__OLD_UTS_LEN);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Only sethostname; gethostname can be implemented by calling uname()
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_sethostname(char *name, int len)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int	i;
	// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if (len > __NEW_UTS_LEN)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	for (i=0; i < len; i++) {
// TODO WGJA WIP: 		if ((system_utsname.nodename[i] = get_fs_byte(name+i)) == 0)
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	system_utsname.nodename[i] = 0;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Only setdomainname; getdomainname can be implemented by calling
// TODO WGJA WIP:  * uname()
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_setdomainname(char *name, int len)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int	i;
	// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if (len > __NEW_UTS_LEN)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	for (i=0; i < len; i++) {
// TODO WGJA WIP: 		if ((system_utsname.domainname[i] = get_fs_byte(name+i)) == 0)
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	system_utsname.domainname[i] = 0;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getrlimit(unsigned int resource, struct rlimit *rlim)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (resource >= RLIM_NLIMITS)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE,rlim,sizeof *rlim);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	put_fs_long(current->rlim[resource].rlim_cur, 
// TODO WGJA WIP: 		    (unsigned long *) rlim);
// TODO WGJA WIP: 	put_fs_long(current->rlim[resource].rlim_max, 
// TODO WGJA WIP: 		    ((unsigned long *) rlim)+1);
// TODO WGJA WIP: 	return 0;	
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_setrlimit(unsigned int resource, struct rlimit *rlim)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct rlimit new_rlim, *old_rlim;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (resource >= RLIM_NLIMITS)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	old_rlim = current->rlim + resource;
// TODO WGJA WIP: 	new_rlim.rlim_cur = get_fs_long((unsigned long *) rlim);
// TODO WGJA WIP: 	new_rlim.rlim_max = get_fs_long(((unsigned long *) rlim)+1);
// TODO WGJA WIP: 	if (((new_rlim.rlim_cur > old_rlim->rlim_max) ||
// TODO WGJA WIP: 	     (new_rlim.rlim_max > old_rlim->rlim_max)) &&
// TODO WGJA WIP: 	    !suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	*old_rlim = new_rlim;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * It would make sense to put struct rusuage in the task_struct,
// TODO WGJA WIP:  * except that would make the task_struct be *really big*.  After
// TODO WGJA WIP:  * task_struct gets moved into malloc'ed memory, it would
// TODO WGJA WIP:  * make sense to do this.  It will make moving the rest of the information
// TODO WGJA WIP:  * a lot simpler!  (Which we're not doing right now because we're not
// TODO WGJA WIP:  * measuring them yet).
// TODO WGJA WIP:  */
// TODO WGJA WIP: int getrusage(struct task_struct *p, int who, struct rusage *ru)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	struct rusage r;
// TODO WGJA WIP: 	unsigned long	*lp, *lpend, *dest;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE, ru, sizeof *ru);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	memset((char *) &r, 0, sizeof(r));
// TODO WGJA WIP: 	switch (who) {
// TODO WGJA WIP: 		case RUSAGE_SELF:
// TODO WGJA WIP: 			r.ru_utime.tv_sec = CT_TO_SECS(p->utime);
// TODO WGJA WIP: 			r.ru_utime.tv_usec = CT_TO_USECS(p->utime);
// TODO WGJA WIP: 			r.ru_stime.tv_sec = CT_TO_SECS(p->stime);
// TODO WGJA WIP: 			r.ru_stime.tv_usec = CT_TO_USECS(p->stime);
// TODO WGJA WIP: 			r.ru_minflt = p->min_flt;
// TODO WGJA WIP: 			r.ru_majflt = p->maj_flt;
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		case RUSAGE_CHILDREN:
// TODO WGJA WIP: 			r.ru_utime.tv_sec = CT_TO_SECS(p->cutime);
// TODO WGJA WIP: 			r.ru_utime.tv_usec = CT_TO_USECS(p->cutime);
// TODO WGJA WIP: 			r.ru_stime.tv_sec = CT_TO_SECS(p->cstime);
// TODO WGJA WIP: 			r.ru_stime.tv_usec = CT_TO_USECS(p->cstime);
// TODO WGJA WIP: 			r.ru_minflt = p->cmin_flt;
// TODO WGJA WIP: 			r.ru_majflt = p->cmaj_flt;
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		default:
// TODO WGJA WIP: 			r.ru_utime.tv_sec = CT_TO_SECS(p->utime + p->cutime);
// TODO WGJA WIP: 			r.ru_utime.tv_usec = CT_TO_USECS(p->utime + p->cutime);
// TODO WGJA WIP: 			r.ru_stime.tv_sec = CT_TO_SECS(p->stime + p->cstime);
// TODO WGJA WIP: 			r.ru_stime.tv_usec = CT_TO_USECS(p->stime + p->cstime);
// TODO WGJA WIP: 			r.ru_minflt = p->min_flt + p->cmin_flt;
// TODO WGJA WIP: 			r.ru_majflt = p->maj_flt + p->cmaj_flt;
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	lp = (unsigned long *) &r;
// TODO WGJA WIP: 	lpend = (unsigned long *) (&r+1);
// TODO WGJA WIP: 	dest = (unsigned long *) ru;
// TODO WGJA WIP: 	for (; lp < lpend; lp++, dest++) 
// TODO WGJA WIP: 		put_fs_long(*lp, dest);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getrusage(int who, struct rusage *ru)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (who != RUSAGE_SELF && who != RUSAGE_CHILDREN)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	return getrusage(current, who, ru);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: #define LATCH ((1193180 + HZ/2)/HZ)
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This version of gettimeofday has near microsecond resolution.
// TODO WGJA WIP:  * It was inspired by Steve McCanne's microtime-i386 for BSD.  -- jrs
// TODO WGJA WIP:  */
// TODO WGJA WIP: static inline void do_gettimeofday(struct timeval *tv)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long nowtime;
// TODO WGJA WIP: 	long count;
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifdef __i386__
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	/* timer count may underflow right here */
// TODO WGJA WIP: 	outb_p(0x00, 0x43);	/* latch the count ASAP */
// TODO WGJA WIP: 	nowtime = jiffies;	/* must be saved inside cli/sti */
// TODO WGJA WIP: 	count = inb_p(0x40);	/* read the latched count */
// TODO WGJA WIP: 	count |= inb_p(0x40) << 8;
// TODO WGJA WIP: 	/* we know probability of underflow is always MUCH less than 1% */
// TODO WGJA WIP: 	if (count < (LATCH - LATCH/100))
// TODO WGJA WIP: 		sti();
// TODO WGJA WIP: 	else {
// TODO WGJA WIP: 		/* check for pending timer interrupt */
// TODO WGJA WIP: 		outb_p(0x0a, 0x20);
// TODO WGJA WIP: 		if (inb(0x20) & 1)
// TODO WGJA WIP: 			nowtime++;
// TODO WGJA WIP: 		sti();
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	nowtime += jiffies_offset;
// TODO WGJA WIP: 	tv->tv_sec = startup_time + CT_TO_SECS(nowtime);
// TODO WGJA WIP: 	/* the correction term is always in the range [0, 1) clocktick */
// TODO WGJA WIP: 	tv->tv_usec = CT_TO_USECS(nowtime)
// TODO WGJA WIP: 		+ ((LATCH - 1) - count)*(1000000/HZ)/LATCH;
// TODO WGJA WIP: #else /* not __i386__ */
// TODO WGJA WIP: 	nowtime = jiffies + jiffes_offset;
// TODO WGJA WIP: 	tv->tv_sec = startup_time + CT_TO_SECS(nowtime);
// TODO WGJA WIP: 	tv->tv_usec = CT_TO_USECS(nowtime);
// TODO WGJA WIP: #endif /* not __i386__ */
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (tv) {
// TODO WGJA WIP: 		struct timeval ktv;
// TODO WGJA WIP: 		error = verify_area(VERIFY_WRITE, tv, sizeof *tv);
// TODO WGJA WIP: 		if (error)
// TODO WGJA WIP: 			return error;
// TODO WGJA WIP: 		do_gettimeofday(&ktv);
// TODO WGJA WIP: 		put_fs_long(ktv.tv_sec, (unsigned long *) &tv->tv_sec);
// TODO WGJA WIP: 		put_fs_long(ktv.tv_usec, (unsigned long *) &tv->tv_usec);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (tz) {
// TODO WGJA WIP: 		error = verify_area(VERIFY_WRITE, tz, sizeof *tz);
// TODO WGJA WIP: 		if (error)
// TODO WGJA WIP: 			return error;
// TODO WGJA WIP: 		put_fs_long(sys_tz.tz_minuteswest, (unsigned long *) tz);
// TODO WGJA WIP: 		put_fs_long(sys_tz.tz_dsttime, ((unsigned long *) tz)+1);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * The first time we set the timezone, we will warp the clock so that
// TODO WGJA WIP:  * it is ticking GMT time instead of local time.  Presumably, 
// TODO WGJA WIP:  * if someone is setting the timezone then we are running in an
// TODO WGJA WIP:  * environment where the programs understand about timezones.
// TODO WGJA WIP:  * This should be done at boot time in the /etc/rc script, as
// TODO WGJA WIP:  * soon as possible, so that the clock can be set right.  Otherwise,
// TODO WGJA WIP:  * various programs will get confused when the clock gets warped.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_settimeofday(struct timeval *tv, struct timezone *tz)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	static int	firsttime = 1;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if (tz) {
// TODO WGJA WIP: 		sys_tz.tz_minuteswest = get_fs_long((unsigned long *) tz);
// TODO WGJA WIP: 		sys_tz.tz_dsttime = get_fs_long(((unsigned long *) tz)+1);
// TODO WGJA WIP: 		if (firsttime) {
// TODO WGJA WIP: 			firsttime = 0;
// TODO WGJA WIP: 			if (!tv)
// TODO WGJA WIP: 				adjust_clock();
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (tv) {
// TODO WGJA WIP: 		int sec, usec;
// TODO WGJA WIP: 
// TODO WGJA WIP: 		sec = get_fs_long((unsigned long *)tv);
// TODO WGJA WIP: 		usec = get_fs_long(((unsigned long *)tv)+1);
	// TODO WGJA WIP: 
// TODO WGJA WIP: 		startup_time = sec - jiffies/HZ;
// TODO WGJA WIP: 		jiffies_offset = usec * HZ / 1000000 - jiffies%HZ;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Adjust the time obtained from the CMOS to be GMT time instead of
// TODO WGJA WIP:  * local time.
// TODO WGJA WIP:  * 
// TODO WGJA WIP:  * This is ugly, but preferable to the alternatives.  Otherwise we
// TODO WGJA WIP:  * would either need to write a program to do it in /etc/rc (and risk
// TODO WGJA WIP:  * confusion if the program gets run more than once; it would also be 
// TODO WGJA WIP:  * hard to make the program warp the clock precisely n hours)  or
// TODO WGJA WIP:  * compile in the timezone information into the kernel.  Bad, bad....
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * XXX Currently does not adjust for daylight savings time.  May not
// TODO WGJA WIP:  * need to do anything, depending on how smart (dumb?) the BIOS
// TODO WGJA WIP:  * is.  Blast it all.... the best thing to do not depend on the CMOS
// TODO WGJA WIP:  * clock at all, but get the time via NTP or timed if you're on a 
// TODO WGJA WIP:  * network....				- TYT, 1/1/92
// TODO WGJA WIP:  */
// TODO WGJA WIP: void adjust_clock(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	startup_time += sys_tz.tz_minuteswest*60;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_umask(int mask)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int old = current->umask;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	current->umask = mask & S_IRWXUGO;
// TODO WGJA WIP: 	return (old);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
