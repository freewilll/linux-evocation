/*
 *  linux/kernel/exit.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#define DEBUG_PROC_TREE

#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/resource.h>
#include <linux/mm.h>
#include <linux/tty.h>

#include <asm/segment.h>
// TODO WGJA WIP: extern void shm_exit (void);
// TODO WGJA WIP: extern void sem_exit (void);
// TODO WGJA WIP: 
// TODO WGJA WIP: int getrusage(struct task_struct *, int, struct rusage *);
// TODO WGJA WIP: 
// TODO WGJA WIP: static int generate(unsigned long sig, struct task_struct * p)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long mask = 1 << (sig-1);
// TODO WGJA WIP: 	struct sigaction * sa = sig + p->sigaction - 1;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* always generate signals for traced processes ??? */
// TODO WGJA WIP: 	if (p->flags & PF_PTRACED) {
// TODO WGJA WIP: 		p->signal |= mask;
// TODO WGJA WIP: 		return 1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	/* don't bother with ignored signals (but SIGCHLD is special) */
// TODO WGJA WIP: 	if (sa->sa_handler == SIG_IGN && sig != SIGCHLD)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	/* some signals are ignored by default.. (but SIGCONT already did its deed) */
// TODO WGJA WIP: 	if ((sa->sa_handler == SIG_DFL) &&
// TODO WGJA WIP: 	    (sig == SIGCONT || sig == SIGCHLD || sig == SIGWINCH))
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	p->signal |= mask;
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int send_sig(unsigned long sig,struct task_struct * p,int priv)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (!p || sig > 32)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	if (!priv && ((sig != SIGCONT) || (current->session != p->session)) &&
// TODO WGJA WIP: 	    (current->euid != p->euid) && (current->uid != p->uid) && !suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if (!sig)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if ((sig == SIGKILL) || (sig == SIGCONT)) {
// TODO WGJA WIP: 		if (p->state == TASK_STOPPED)
// TODO WGJA WIP: 			p->state = TASK_RUNNING;
// TODO WGJA WIP: 		p->exit_code = 0;
// TODO WGJA WIP: 		p->signal &= ~( (1<<(SIGSTOP-1)) | (1<<(SIGTSTP-1)) |
// TODO WGJA WIP: 				(1<<(SIGTTIN-1)) | (1<<(SIGTTOU-1)) );
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	/* Depends on order SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU */
// TODO WGJA WIP: 	if ((sig >= SIGSTOP) && (sig <= SIGTTOU)) 
// TODO WGJA WIP: 		p->signal &= ~(1<<(SIGCONT-1));
// TODO WGJA WIP: 	/* Actually generate the signal */
// TODO WGJA WIP: 	generate(sig,p);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void notify_parent(struct task_struct * tsk)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (tsk->p_pptr == task[1])
// TODO WGJA WIP: 		tsk->exit_signal = SIGCHLD;
// TODO WGJA WIP: 	send_sig(tsk->exit_signal, tsk->p_pptr, 1);
// TODO WGJA WIP: 	wake_up_interruptible(&tsk->p_pptr->wait_chldexit);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void release(struct task_struct * p)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!p)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (p == current) {
// TODO WGJA WIP: 		printk("task releasing itself\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	for (i=1 ; i<NR_TASKS ; i++)
// TODO WGJA WIP: 		if (task[i] == p) {
// TODO WGJA WIP: 			task[i] = NULL;
// TODO WGJA WIP: 			REMOVE_LINKS(p);
// TODO WGJA WIP: 			free_page(p->kernel_stack_page);
// TODO WGJA WIP: 			free_page((long) p);
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	panic("trying to release non-existent task");
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifdef DEBUG_PROC_TREE
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Check to see if a task_struct pointer is present in the task[] array
// TODO WGJA WIP:  * Return 0 if found, and 1 if not found.
// TODO WGJA WIP:  */
// TODO WGJA WIP: int bad_task_ptr(struct task_struct *p)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int 	i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!p)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	for (i=0 ; i<NR_TASKS ; i++)
// TODO WGJA WIP: 		if (task[i] == p)
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }
	// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This routine scans the pid tree and make sure the rep invarient still
// TODO WGJA WIP:  * holds.  Used for debugging only, since it's very slow....
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * It looks a lot scarier than it really is.... we're doing nothing more
// TODO WGJA WIP:  * than verifying the doubly-linked list found in p_ysptr and p_osptr, 
// TODO WGJA WIP:  * and checking it corresponds with the process tree defined by p_cptr and 
// TODO WGJA WIP:  * p_pptr;
// TODO WGJA WIP:  */
// TODO WGJA WIP: void audit_ptree(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int	i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for (i=1 ; i<NR_TASKS ; i++) {
// TODO WGJA WIP: 		if (!task[i])
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (bad_task_ptr(task[i]->p_pptr))
// TODO WGJA WIP: 			printk("Warning, pid %d's parent link is bad\n",
// TODO WGJA WIP: 				task[i]->pid);
// TODO WGJA WIP: 		if (bad_task_ptr(task[i]->p_cptr))
// TODO WGJA WIP: 			printk("Warning, pid %d's child link is bad\n",
// TODO WGJA WIP: 				task[i]->pid);
// TODO WGJA WIP: 		if (bad_task_ptr(task[i]->p_ysptr))
// TODO WGJA WIP: 			printk("Warning, pid %d's ys link is bad\n",
// TODO WGJA WIP: 				task[i]->pid);
// TODO WGJA WIP: 		if (bad_task_ptr(task[i]->p_osptr))
// TODO WGJA WIP: 			printk("Warning, pid %d's os link is bad\n",
// TODO WGJA WIP: 				task[i]->pid);
// TODO WGJA WIP: 		if (task[i]->p_pptr == task[i])
// TODO WGJA WIP: 			printk("Warning, pid %d parent link points to self\n",
// TODO WGJA WIP: 				task[i]->pid);
// TODO WGJA WIP: 		if (task[i]->p_cptr == task[i])
// TODO WGJA WIP: 			printk("Warning, pid %d child link points to self\n",
// TODO WGJA WIP: 				task[i]->pid);
// TODO WGJA WIP: 		if (task[i]->p_ysptr == task[i])
// TODO WGJA WIP: 			printk("Warning, pid %d ys link points to self\n",
// TODO WGJA WIP: 				task[i]->pid);
// TODO WGJA WIP: 		if (task[i]->p_osptr == task[i])
// TODO WGJA WIP: 			printk("Warning, pid %d os link points to self\n",
// TODO WGJA WIP: 				task[i]->pid);
// TODO WGJA WIP: 		if (task[i]->p_osptr) {
// TODO WGJA WIP: 			if (task[i]->p_pptr != task[i]->p_osptr->p_pptr)
// TODO WGJA WIP: 				printk(
// TODO WGJA WIP: 			"Warning, pid %d older sibling %d parent is %d\n",
// TODO WGJA WIP: 				task[i]->pid, task[i]->p_osptr->pid,
// TODO WGJA WIP: 				task[i]->p_osptr->p_pptr->pid);
// TODO WGJA WIP: 			if (task[i]->p_osptr->p_ysptr != task[i])
// TODO WGJA WIP: 				printk(
// TODO WGJA WIP: 		"Warning, pid %d older sibling %d has mismatched ys link\n",
// TODO WGJA WIP: 				task[i]->pid, task[i]->p_osptr->pid);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (task[i]->p_ysptr) {
// TODO WGJA WIP: 			if (task[i]->p_pptr != task[i]->p_ysptr->p_pptr)
// TODO WGJA WIP: 				printk(
// TODO WGJA WIP: 			"Warning, pid %d younger sibling %d parent is %d\n",
// TODO WGJA WIP: 				task[i]->pid, task[i]->p_osptr->pid,
// TODO WGJA WIP: 				task[i]->p_osptr->p_pptr->pid);
// TODO WGJA WIP: 			if (task[i]->p_ysptr->p_osptr != task[i])
// TODO WGJA WIP: 				printk(
// TODO WGJA WIP: 		"Warning, pid %d younger sibling %d has mismatched os link\n",
// TODO WGJA WIP: 				task[i]->pid, task[i]->p_ysptr->pid);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (task[i]->p_cptr) {
// TODO WGJA WIP: 			if (task[i]->p_cptr->p_pptr != task[i])
// TODO WGJA WIP: 				printk(
// TODO WGJA WIP: 			"Warning, pid %d youngest child %d has mismatched parent link\n",
// TODO WGJA WIP: 				task[i]->pid, task[i]->p_cptr->pid);
// TODO WGJA WIP: 			if (task[i]->p_cptr->p_ysptr)
// TODO WGJA WIP: 				printk(
// TODO WGJA WIP: 			"Warning, pid %d youngest child %d has non-NULL ys link\n",
// TODO WGJA WIP: 				task[i]->pid, task[i]->p_cptr->pid);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: #endif /* DEBUG_PROC_TREE */

/*
 * This checks not only the pgrp, but falls back on the pid if no
 * satisfactory prgp is found. I dunno - gdb doesn't work correctly
 * without this...
 */
int session_of_pgrp(int pgrp)
{
	struct task_struct *p;
	int fallback;

	fallback = -1;
	for_each_task(p) {
 		if (p->session <= 0)
 			continue;
		if (p->pgrp == pgrp)
			return p->session;
		if (p->pid == pgrp)
			fallback = p->session;
	}
	return fallback;
}

/*
 * kill_pg() sends a signal to a process group: this is what the tty
 * control characters do (^C, ^Z etc)
 */
int kill_pg(int pgrp, int sig, int priv)
{
	struct task_struct *p;
	int err,retval = -ESRCH;
	int found = 0;

	if (sig<0 || sig>32 || pgrp<=0)
		return -EINVAL;
	for_each_task(p) {
		if (p->pgrp == pgrp) {
			if ((err = send_sig(sig,p,priv)) != 0)
				retval = err;
			else
				found++;
		}
	}
	return(found ? 0 : retval);
}

/*
 * kill_sl() sends a signal to the session leader: this is used
 * to send SIGHUP to the controlling process of a terminal when
 * the connection is lost.
 */
int kill_sl(int sess, int sig, int priv)
{
	struct task_struct *p;
	int err,retval = -ESRCH;
	int found = 0;

	if (sig<0 || sig>32 || sess<=0)
		return -EINVAL;
	for_each_task(p) {
		if (p->session == sess && p->leader) {
			if ((err = send_sig(sig,p,priv)) != 0)
				retval = err;
			else
				found++;
		}
	}
	return(found ? 0 : retval);
}

int kill_proc(int pid, int sig, int priv)
{
 	struct task_struct *p;

	if (sig<0 || sig>32)
		return -EINVAL;
	for_each_task(p) {
		if (p && p->pid == pid)
			return send_sig(sig,p,priv);
	}
	return(-ESRCH);
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * POSIX specifies that kill(-1,sig) is unspecified, but what we have
// TODO WGJA WIP:  * is probably wrong.  Should make it like BSD or SYSV.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_kill(int pid,int sig)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int err, retval = 0, count = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!pid)
// TODO WGJA WIP: 		return(kill_pg(current->pgrp,sig,0));
// TODO WGJA WIP: 	if (pid == -1) {
// TODO WGJA WIP: 		struct task_struct * p;
// TODO WGJA WIP: 		for_each_task(p) {
// TODO WGJA WIP: 			if (p->pid > 1 && p != current) {
// TODO WGJA WIP: 				++count;
// TODO WGJA WIP: 				if ((err = send_sig(sig,p,0)) != -EPERM)
// TODO WGJA WIP: 					retval = err;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		return(count ? retval : -ESRCH);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (pid < 0) 
// TODO WGJA WIP: 		return(kill_pg(-pid,sig,0));
// TODO WGJA WIP: 	/* Normal kill */
// TODO WGJA WIP: 	return(kill_proc(pid,sig,0));
// TODO WGJA WIP: }

/*
 * Determine if a process group is "orphaned", according to the POSIX
 * definition in 2.2.2.52.  Orphaned process groups are not to be affected
 * by terminal-generated stop signals.  Newly orphaned process groups are 
 * to receive a SIGHUP and a SIGCONT.
 * 
 * "I ask you, have you ever known what it is to be an orphan?"
 */
int is_orphaned_pgrp(int pgrp)
{
	struct task_struct *p;

	for_each_task(p) {
		if ((p->pgrp != pgrp) || 
		    (p->state == TASK_ZOMBIE) ||
		    (p->p_pptr->pid == 1))
			continue;
		if ((p->p_pptr->pgrp != pgrp) &&
		    (p->p_pptr->session == p->session))
			return 0;
	}
	return(1);	/* (sighing) "Often!" */
}

// TODO WGJA WIP: static int has_stopped_jobs(int pgrp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct task_struct * p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for_each_task(p) {
// TODO WGJA WIP: 		if (p->pgrp != pgrp)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (p->state == TASK_STOPPED)
// TODO WGJA WIP: 			return(1);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return(0);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static void forget_original_parent(struct task_struct * father)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct task_struct * p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for_each_task(p) {
// TODO WGJA WIP: 		if (p->p_opptr == father)
// TODO WGJA WIP: 			if (task[1])
// TODO WGJA WIP: 				p->p_opptr = task[1];
// TODO WGJA WIP: 			else
// TODO WGJA WIP: 				p->p_opptr = task[0];
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: volatile void do_exit(long code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct task_struct *p;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: fake_volatile:
// TODO WGJA WIP: 	if (current->semun)
// TODO WGJA WIP: 		sem_exit();
// TODO WGJA WIP: 	if (current->shm)
// TODO WGJA WIP: 		shm_exit();
// TODO WGJA WIP: 	free_page_tables(current);
// TODO WGJA WIP: 	for (i=0 ; i<NR_OPEN ; i++)
// TODO WGJA WIP: 		if (current->filp[i])
// TODO WGJA WIP: 			sys_close(i);
// TODO WGJA WIP: 	forget_original_parent(current);
// TODO WGJA WIP: 	iput(current->pwd);
// TODO WGJA WIP: 	current->pwd = NULL;
// TODO WGJA WIP: 	iput(current->root);
// TODO WGJA WIP: 	current->root = NULL;
// TODO WGJA WIP: 	iput(current->executable);
// TODO WGJA WIP: 	current->executable = NULL;
// TODO WGJA WIP: 	/* Release all of the old mmap stuff. */
	// TODO WGJA WIP: 
// TODO WGJA WIP: 	{
// TODO WGJA WIP: 		struct vm_area_struct * mpnt, *mpnt1;
// TODO WGJA WIP: 		mpnt = current->mmap;
// TODO WGJA WIP: 		current->mmap = NULL;
// TODO WGJA WIP: 		while (mpnt) {
// TODO WGJA WIP: 			mpnt1 = mpnt->vm_next;
// TODO WGJA WIP: 			if (mpnt->vm_ops->close)
// TODO WGJA WIP: 				mpnt->vm_ops->close(mpnt);
// TODO WGJA WIP: 			kfree(mpnt);
// TODO WGJA WIP: 			mpnt = mpnt1;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (current->ldt) {
// TODO WGJA WIP: 		free_page((unsigned long) current->ldt);
// TODO WGJA WIP: 		current->ldt = NULL;
// TODO WGJA WIP: 		for (i=1 ; i<NR_TASKS ; i++) {
// TODO WGJA WIP: 			if (task[i] == current) {
// TODO WGJA WIP: 				set_ldt_desc(gdt+(i<<1)+FIRST_LDT_ENTRY, &default_ldt, 1);
// TODO WGJA WIP: 				load_ldt(i);
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	current->state = TASK_ZOMBIE;
// TODO WGJA WIP: 	current->exit_code = code;
// TODO WGJA WIP: 	current->rss = 0;
// TODO WGJA WIP: 	/* 
// TODO WGJA WIP: 	 * Check to see if any process groups have become orphaned
// TODO WGJA WIP: 	 * as a result of our exiting, and if they have any stopped
// TODO WGJA WIP: 	 * jobs, send them a SIGUP and then a SIGCONT.  (POSIX 3.2.2.2)
// TODO WGJA WIP: 	 *
// TODO WGJA WIP: 	 * Case i: Our father is in a different pgrp than we are
// TODO WGJA WIP: 	 * and we were the only connection outside, so our pgrp
// TODO WGJA WIP: 	 * is about to become orphaned.
// TODO WGJA WIP:  	 */
// TODO WGJA WIP: 	if ((current->p_pptr->pgrp != current->pgrp) &&
// TODO WGJA WIP: 	    (current->p_pptr->session == current->session) &&
// TODO WGJA WIP: 	    is_orphaned_pgrp(current->pgrp) &&
// TODO WGJA WIP: 	    has_stopped_jobs(current->pgrp)) {
// TODO WGJA WIP: 		kill_pg(current->pgrp,SIGHUP,1);
// TODO WGJA WIP: 		kill_pg(current->pgrp,SIGCONT,1);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	/* Let father know we died */
// TODO WGJA WIP: 	notify_parent(current);
	// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * This loop does two things:
// TODO WGJA WIP: 	 * 
// TODO WGJA WIP:   	 * A.  Make init inherit all the child processes
// TODO WGJA WIP: 	 * B.  Check to see if any process groups have become orphaned
// TODO WGJA WIP: 	 *	as a result of our exiting, and if they have any stopped
// TODO WGJA WIP: 	 *	jobs, send them a SIGHUP and then a SIGCONT.  (POSIX 3.2.2.2)
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	while ((p = current->p_cptr) != NULL) {
// TODO WGJA WIP: 		current->p_cptr = p->p_osptr;
// TODO WGJA WIP: 		p->p_ysptr = NULL;
// TODO WGJA WIP: 		p->flags &= ~(PF_PTRACED|PF_TRACESYS);
// TODO WGJA WIP: 		if (task[1])
// TODO WGJA WIP: 			p->p_pptr = task[1];
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			p->p_pptr = task[0];
// TODO WGJA WIP: 		p->p_osptr = p->p_pptr->p_cptr;
// TODO WGJA WIP: 		p->p_osptr->p_ysptr = p;
// TODO WGJA WIP: 		p->p_pptr->p_cptr = p;
// TODO WGJA WIP: 		if (p->state == TASK_ZOMBIE)
// TODO WGJA WIP: 			notify_parent(p);
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * process group orphan check
// TODO WGJA WIP: 		 * Case ii: Our child is in a different pgrp 
// TODO WGJA WIP: 		 * than we are, and it was the only connection
// TODO WGJA WIP: 		 * outside, so the child pgrp is now orphaned.
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		if ((p->pgrp != current->pgrp) &&
// TODO WGJA WIP: 		    (p->session == current->session) &&
// TODO WGJA WIP: 		    is_orphaned_pgrp(p->pgrp) &&
// TODO WGJA WIP: 		    has_stopped_jobs(p->pgrp)) {
// TODO WGJA WIP: 			kill_pg(p->pgrp,SIGHUP,1);
// TODO WGJA WIP: 			kill_pg(p->pgrp,SIGCONT,1);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (current->leader) {
// TODO WGJA WIP: 		struct task_struct *p;
// TODO WGJA WIP: 		struct tty_struct *tty;
// TODO WGJA WIP: 
// TODO WGJA WIP: 		if (current->tty >= 0) {
// TODO WGJA WIP: 			tty = TTY_TABLE(current->tty);
// TODO WGJA WIP: 			if (tty) {
// TODO WGJA WIP: 				if (tty->pgrp > 0)
// TODO WGJA WIP: 					kill_pg(tty->pgrp, SIGHUP, 1);
// TODO WGJA WIP: 				tty->pgrp = -1;
// TODO WGJA WIP: 				tty->session = 0;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		for_each_task(p) {
// TODO WGJA WIP: 			if (p->session == current->session)
// TODO WGJA WIP: 				p->tty = -1;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (last_task_used_math == current)
// TODO WGJA WIP: 		last_task_used_math = NULL;
// TODO WGJA WIP: #ifdef DEBUG_PROC_TREE
// TODO WGJA WIP: 	audit_ptree();
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	schedule();
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * In order to get rid of the "volatile function does return" message
// TODO WGJA WIP:  * I did this little loop that confuses gcc to think do_exit really
// TODO WGJA WIP:  * is volatile. In fact it's schedule() that is volatile in some
// TODO WGJA WIP:  * circumstances: when current->state = ZOMBIE, schedule() never
// TODO WGJA WIP:  * returns.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * In fact the natural way to do all this is to have the label and the
// TODO WGJA WIP:  * goto right after each other, but I put the fake_volatile label at
// TODO WGJA WIP:  * the start of the function just in case something /really/ bad
// TODO WGJA WIP:  * happens, and the schedule returns. This way we can try again. I'm
// TODO WGJA WIP:  * not paranoid: it's just that everybody is out to get me.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 	goto fake_volatile;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_exit(int error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	do_exit((error_code&0xff)<<8);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_wait4(pid_t pid,unsigned long * stat_addr, int options, struct rusage * ru)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int flag, retval;
// TODO WGJA WIP: 	struct wait_queue wait = { current, NULL };
// TODO WGJA WIP: 	struct task_struct *p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (stat_addr) {
// TODO WGJA WIP: 		flag = verify_area(VERIFY_WRITE, stat_addr, 4);
// TODO WGJA WIP: 		if (flag)
// TODO WGJA WIP: 			return flag;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	add_wait_queue(&current->wait_chldexit,&wait);
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 	flag=0;
// TODO WGJA WIP:  	for (p = current->p_cptr ; p ; p = p->p_osptr) {
// TODO WGJA WIP: 		if (pid>0) {
// TODO WGJA WIP: 			if (p->pid != pid)
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 		} else if (!pid) {
// TODO WGJA WIP: 			if (p->pgrp != current->pgrp)
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 		} else if (pid != -1) {
// TODO WGJA WIP: 			if (p->pgrp != -pid)
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		/* wait for cloned processes iff the __WCLONE flag is set */
// TODO WGJA WIP: 		if ((p->exit_signal != SIGCHLD) ^ ((options & __WCLONE) != 0))
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		flag = 1;
// TODO WGJA WIP: 		switch (p->state) {
// TODO WGJA WIP: 			case TASK_STOPPED:
// TODO WGJA WIP: 				if (!p->exit_code)
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				if (!(options & WUNTRACED) && !(p->flags & PF_PTRACED))
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				if (stat_addr)
// TODO WGJA WIP: 					put_fs_long((p->exit_code << 8) | 0x7f,
// TODO WGJA WIP: 						stat_addr);
// TODO WGJA WIP: 				p->exit_code = 0;
// TODO WGJA WIP: 				if (ru != NULL)
// TODO WGJA WIP: 					getrusage(p, RUSAGE_BOTH, ru);
// TODO WGJA WIP: 				retval = p->pid;
// TODO WGJA WIP: 				goto end_wait4;
// TODO WGJA WIP: 			case TASK_ZOMBIE:
// TODO WGJA WIP: 				current->cutime += p->utime + p->cutime;
// TODO WGJA WIP: 				current->cstime += p->stime + p->cstime;
// TODO WGJA WIP: 				current->cmin_flt += p->min_flt + p->cmin_flt;
// TODO WGJA WIP: 				current->cmaj_flt += p->maj_flt + p->cmaj_flt;
// TODO WGJA WIP: 				if (ru != NULL)
// TODO WGJA WIP: 					getrusage(p, RUSAGE_BOTH, ru);
// TODO WGJA WIP: 				flag = p->pid;
// TODO WGJA WIP: 				if (stat_addr)
// TODO WGJA WIP: 					put_fs_long(p->exit_code, stat_addr);
// TODO WGJA WIP: 				if (p->p_opptr != p->p_pptr) {
// TODO WGJA WIP: 					REMOVE_LINKS(p);
// TODO WGJA WIP: 					p->p_pptr = p->p_opptr;
// TODO WGJA WIP: 					SET_LINKS(p);
// TODO WGJA WIP: 					notify_parent(p);
// TODO WGJA WIP: 				} else
// TODO WGJA WIP: 					release(p);
// TODO WGJA WIP: #ifdef DEBUG_PROC_TREE
// TODO WGJA WIP: 				audit_ptree();
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 				retval = flag;
// TODO WGJA WIP: 				goto end_wait4;
// TODO WGJA WIP: 			default:
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (flag) {
// TODO WGJA WIP: 		retval = 0;
// TODO WGJA WIP: 		if (options & WNOHANG)
// TODO WGJA WIP: 			goto end_wait4;
// TODO WGJA WIP: 		current->state=TASK_INTERRUPTIBLE;
// TODO WGJA WIP: 		schedule();
// TODO WGJA WIP: 		current->signal &= ~(1<<(SIGCHLD-1));
// TODO WGJA WIP: 		retval = -ERESTARTSYS;
// TODO WGJA WIP: 		if (current->signal & ~current->blocked)
// TODO WGJA WIP: 			goto end_wait4;
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	retval = -ECHILD;
// TODO WGJA WIP: end_wait4:
// TODO WGJA WIP: 	remove_wait_queue(&current->wait_chldexit,&wait);
// TODO WGJA WIP: 	return retval;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * sys_waitpid() remains for compatibility. waitpid() should be
// TODO WGJA WIP:  * implemented by calling sys_wait4() from libc.a.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_waitpid(pid_t pid,unsigned long * stat_addr, int options)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return sys_wait4(pid, stat_addr, options, NULL);
// TODO WGJA WIP: }
