/*
 *  linux/kernel/sched.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * 'sched.c' is the main kernel file. It contains scheduling primitives
 * (sleep_on, wakeup, schedule etc) as well as a number of simple system
 * call functions (type getpid(), which just extracts a field from
 * current-task
 */

#include <linux/config.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/page.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/sys.h>
// TODO WGJA WIP: #include <linux/fdreg.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/ptrace.h>
#include <linux/segment.h>
// TODO WGJA WIP: #include <linux/delay.h>
// TODO WGJA WIP: 
#include <asm/system.h>
#include <asm/io.h>
#include <asm/system.h>

#define TIMER_IRQ 0

int need_resched = 0;

/*
 * Tell us the machine setup..
 */
int hard_math = 0;		/* set by boot/head.S */
int x86 = 0;			/* set by boot/head.S to 3 or 4 */
int ignore_irq13 = 0;		/* set if exception 16 works */
// TODO WGJA WIP: int wp_works_ok = 0;		/* not used currently */
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int _setitimer(int, struct itimerval *, struct itimerval *);
// TODO WGJA WIP: unsigned long * prof_buffer = NULL;
// TODO WGJA WIP: unsigned long prof_len = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: #define _S(nr) (1<<((nr)-1))

#define LATCH ((1193180 + HZ/2)/HZ)

extern void mem_use(void);

extern int timer_interrupt(void);
extern "C" int system_call(void);

static unsigned long init_kernel_stack[1024];
struct task_struct init_task = INIT_TASK;

unsigned long volatile jiffies=0;
unsigned long startup_time=0;
int jiffies_offset = 0;		/* # clock ticks to add to get "true
				   time".  Should always be less than
				   1 second's worth.  For time fanatics
				   who like to syncronize their machines
				   to WWV :-) */

struct task_struct *current = &init_task;
struct task_struct *last_task_used_math = (task_struct *) NULL;

struct task_struct * task[NR_TASKS] = {&init_task, };

long user_stack [ PAGE_SIZE>>2 ] ;

struct stack_struct {
	long * a;
	short b;
	};
stack_struct stack_start = { & user_stack [PAGE_SIZE>>2] , KERNEL_DS };
// TODO WGJA WIP: /*
// TODO WGJA WIP:  *  'math_state_restore()' saves the current math information in the
// TODO WGJA WIP:  * old math state array, and gets the new ones from the current task
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Careful.. There are problems with IBM-designed IRQ13 behaviour.
// TODO WGJA WIP:  * Don't touch unless you *really* know how it works.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" void math_state_restore(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	__asm__ __volatile__("clts");
// TODO WGJA WIP: 	if (last_task_used_math == current)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	timer_table[COPRO_TIMER].expires = jiffies+50;
// TODO WGJA WIP: 	timer_active |= 1<<COPRO_TIMER;	
// TODO WGJA WIP: 	if (last_task_used_math)
// TODO WGJA WIP: 		__asm__("fnsave %0":"=m" (last_task_used_math->tss.i387));
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		__asm__("fnclex");
// TODO WGJA WIP: 	last_task_used_math = current;
// TODO WGJA WIP: 	if (current->used_math) {
// TODO WGJA WIP: 		__asm__("frstor %0": :"m" (current->tss.i387));
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		__asm__("fninit");
// TODO WGJA WIP: 		current->used_math=1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	timer_active &= ~(1<<COPRO_TIMER);
// TODO WGJA WIP: }

/*
 *  'schedule()' is the scheduler function. It's a very simple and nice
 * scheduler: it's not perfect, but certainly works for most things.
 * The one thing you might take a look at is the signal-handler code here.
 *
 *   NOTE!!  Task 0 is the 'idle' task, which gets called when no other
 * tasks can run. It can not be killed, and it cannot sleep. The 'state'
 * information in task[0] is never used.
 *
 * The "confuse_gcc" goto is used only to get better assembly code..
 * Djikstra probably hates me.
 */

extern "C" void schedule(void)
{
	int c;
	struct task_struct * p;
	struct task_struct * next;

/* check alarm, wake up any interruptible tasks that have got a signal */

	sti();
	need_resched = 0;
	p = &init_task;
	for (;;) {
		if ((p = p->next_task) == &init_task)
			goto confuse_gcc1;
		if (p->state != TASK_INTERRUPTIBLE)
			continue;
		if (p->signal & ~p->blocked) {
			p->state = TASK_RUNNING;
			continue;
		}
		if (p->timeout && p->timeout < jiffies) {
			p->timeout = 0;
			p->state = TASK_RUNNING;
		}
	}
confuse_gcc1:

/* this is the scheduler proper: */

	c = -1;
	next = p = &init_task;
	for (;;) {
		if ((p = p->next_task) == &init_task)
			goto confuse_gcc2;
		if (p->state == TASK_RUNNING && p->counter > c)
			c = p->counter, next = p;
	}
confuse_gcc2:
	if (!c) {
		for_each_task(p)
			p->counter = (p->counter >> 1) + p->priority;
	}
	char *vidmem = (char *)0xb8000;
	vidmem[78 * 2 + 1] ^= 0x80;  // TODO WGJA visual display of aliveness
	switch_to(next);
}

// TODO WGJA WIP: extern "C" int sys_pause(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	current->state = TASK_INTERRUPTIBLE;
// TODO WGJA WIP: 	schedule();
// TODO WGJA WIP: 	return -ERESTARTNOHAND;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * wake_up doesn't wake up stopped processes - they have to be awakened
// TODO WGJA WIP:  * with signals or similar.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Note that this doesn't need cli-sti pairs: interrupts may not change
// TODO WGJA WIP:  * the wait-queue structures directly, but only call wake_up() to wake
// TODO WGJA WIP:  * a process. The process itself must remove the queue once it has woken.
// TODO WGJA WIP:  */
// TODO WGJA WIP: void wake_up(struct wait_queue **q)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct wait_queue *tmp;
// TODO WGJA WIP: 	struct task_struct * p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!q || !(tmp = *q))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	do {
// TODO WGJA WIP: 		if ((p = tmp->task) != NULL) {
// TODO WGJA WIP: 			if ((p->state == TASK_UNINTERRUPTIBLE) ||
// TODO WGJA WIP: 			    (p->state == TASK_INTERRUPTIBLE)) {
// TODO WGJA WIP: 				p->state = TASK_RUNNING;
// TODO WGJA WIP: 				if (p->counter > current->counter)
// TODO WGJA WIP: 					need_resched = 1;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!tmp->next) {
// TODO WGJA WIP: 			printk("wait_queue is bad (eip = %08x)\n",((unsigned long *) q)[-1]);
// TODO WGJA WIP: 			printk("        q = %08x\n",q);
// TODO WGJA WIP: 			printk("       *q = %08x\n",*q);
// TODO WGJA WIP: 			printk("      tmp = %08x\n",tmp);
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		tmp = tmp->next;
// TODO WGJA WIP: 	} while (tmp != *q);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void wake_up_interruptible(struct wait_queue **q)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct wait_queue *tmp;
// TODO WGJA WIP: 	struct task_struct * p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!q || !(tmp = *q))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	do {
// TODO WGJA WIP: 		if ((p = tmp->task) != NULL) {
// TODO WGJA WIP: 			if (p->state == TASK_INTERRUPTIBLE) {
// TODO WGJA WIP: 				p->state = TASK_RUNNING;
// TODO WGJA WIP: 				if (p->counter > current->counter)
// TODO WGJA WIP: 					need_resched = 1;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!tmp->next) {
// TODO WGJA WIP: 			printk("wait_queue is bad (eip = %08x)\n",((unsigned long *) q)[-1]);
// TODO WGJA WIP: 			printk("        q = %08x\n",q);
// TODO WGJA WIP: 			printk("       *q = %08x\n",*q);
// TODO WGJA WIP: 			printk("      tmp = %08x\n",tmp);
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		tmp = tmp->next;
// TODO WGJA WIP: 	} while (tmp != *q);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static inline void __sleep_on(struct wait_queue **p, int state)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 	struct wait_queue wait = { current, NULL };
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!p)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (current == task[0])
// TODO WGJA WIP: 		panic("task[0] trying to sleep");
// TODO WGJA WIP: 	current->state = state;
// TODO WGJA WIP: 	add_wait_queue(p, &wait);
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	sti();
// TODO WGJA WIP: 	schedule();
// TODO WGJA WIP: 	remove_wait_queue(p, &wait);
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void interruptible_sleep_on(struct wait_queue **p)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	__sleep_on(p,TASK_INTERRUPTIBLE);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void sleep_on(struct wait_queue **p)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	__sleep_on(p,TASK_UNINTERRUPTIBLE);
// TODO WGJA WIP: }

static struct timer_list * next_timer = (timer_list *) NULL;

// TODO WGJA WIP: void add_timer(struct timer_list * timer)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 	struct timer_list ** p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!timer)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	timer->next = NULL;
// TODO WGJA WIP: 	p = &next_timer;
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	while (*p) {
// TODO WGJA WIP: 		if ((*p)->expires > timer->expires) {
// TODO WGJA WIP: 			(*p)->expires -= timer->expires;
// TODO WGJA WIP: 			timer->next = *p;
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		timer->expires -= (*p)->expires;
// TODO WGJA WIP: 		p = &(*p)->next;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	*p = timer;
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int del_timer(struct timer_list * timer)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 	unsigned long expires = 0;
// TODO WGJA WIP: 	struct timer_list **p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	p = &next_timer;
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	while (*p) {
// TODO WGJA WIP: 		if (*p == timer) {
// TODO WGJA WIP: 			if ((*p = timer->next) != NULL)
// TODO WGJA WIP: 				(*p)->expires += timer->expires;
// TODO WGJA WIP: 			timer->expires += expires;
// TODO WGJA WIP: 			restore_flags(flags);
// TODO WGJA WIP: 			return 1;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		expires += (*p)->expires;
// TODO WGJA WIP: 		p = &(*p)->next;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }

unsigned long timer_active = 0;
struct timer_struct timer_table[32];

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Hmm.. Changed this, as the GNU make sources (load.c) seems to
// TODO WGJA WIP:  * imply that avenrun[] is the standard name for this kind of thing.
// TODO WGJA WIP:  * Nothing else seems to be standardized: the fractional size etc
// TODO WGJA WIP:  * all seem to differ on different machines.
// TODO WGJA WIP:  */
// TODO WGJA WIP: unsigned long avenrun[3] = { 0,0,0 };
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Nr of active tasks - counted in fixed-point numbers
// TODO WGJA WIP:  */
// TODO WGJA WIP: static unsigned long count_active_tasks(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct task_struct **p;
// TODO WGJA WIP: 	unsigned long nr = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for(p = &LAST_TASK; p > &FIRST_TASK; --p)
// TODO WGJA WIP: 		if (*p && (*p)->state == TASK_RUNNING)
// TODO WGJA WIP: 			nr += FIXED_1;
// TODO WGJA WIP: 	return nr;
// TODO WGJA WIP: }

static inline void calc_load(void)
{
	return; // TODO WGJA calc_load
	// unsigned long active_tasks; /* fixed-point */
	// static int count = LOAD_FREQ;

	// if (count-- > 0)
	// 	return;
	// count = LOAD_FREQ;
	// active_tasks = count_active_tasks();
	// CALC_LOAD(avenrun[0], EXP_1, active_tasks);
	// CALC_LOAD(avenrun[1], EXP_5, active_tasks);
	// CALC_LOAD(avenrun[2], EXP_15, active_tasks);
}

/*
 * The int argument is really a (struct pt_regs *), in case the
 * interrupt wants to know from where it was called. The timer
 * irq uses this to decide if it should update the user or system
 * times.
 */

static void do_timer(struct pt_regs * regs)
{
	unsigned long mask;
	struct timer_struct *tp = timer_table+0;
	struct task_struct * task_p;

	jiffies++;
	calc_load();
	if ((VM_MASK & regs->eflags) || (3 & regs->cs)) {
		current->utime++;
		/* Update ITIMER_VIRT for current task if not in a system call */
		if (current->it_virt_value && !(--current->it_virt_value)) {
			current->it_virt_value = current->it_virt_incr;
			send_sig(SIGVTALRM,current,1);
		}
	} else {
		current->stime++;
#ifdef CONFIG_PROFILE
		if (prof_buffer && current != task[0]) {
			unsigned long eip = regs->eip;
			eip >>= 2;
			if (eip < prof_len)
				prof_buffer[eip]++;
		}
#endif
	}
	if (current == task[0] || (--current->counter)<=0) {
		current->counter=0;
		need_resched = 1;
	}
	/* Update ITIMER_REAL for every task */
	for_each_task(task_p) {
		if (!task_p->it_real_value)
			continue;
		if (--task_p->it_real_value)
			continue;
		send_sig(SIGALRM,task_p,1);
		task_p->it_real_value = task_p->it_real_incr;
		need_resched = 1;
	}
	/* Update ITIMER_PROF for the current task */
	if (current->it_prof_value && !(--current->it_prof_value)) {
		current->it_prof_value = current->it_prof_incr;
		send_sig(SIGPROF,current,1);
	}
	for (mask = 1 ; mask ; tp++,mask += mask) {
		if (mask > timer_active)
			break;
		if (!(mask & timer_active))
			continue;
		if (tp->expires > jiffies)
			continue;
		timer_active &= ~mask;
		tp->fn();
		sti();
	}
	cli();
	while (next_timer && next_timer->expires == 0) {
		void (*fn)(unsigned long) = next_timer->function;
		unsigned long data = next_timer->data;
		next_timer = next_timer->next;
		sti();
		fn(data);
		cli();
	}
	if (next_timer)
		next_timer->expires--;
	sti();
}

// TODO WGJA WIP: extern "C" int sys_alarm(long seconds)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct itimerval it_new, it_old;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	it_new.it_interval.tv_sec = it_new.it_interval.tv_usec = 0;
// TODO WGJA WIP: 	it_new.it_value.tv_sec = seconds;
// TODO WGJA WIP: 	it_new.it_value.tv_usec = 0;
// TODO WGJA WIP: 	_setitimer(ITIMER_REAL, &it_new, &it_old);
// TODO WGJA WIP: 	return(it_old.it_value.tv_sec + (it_old.it_value.tv_usec / 1000000));
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getpid(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return current->pid;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getppid(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return current->p_pptr->pid;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getuid(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return current->uid;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_geteuid(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return current->euid;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getgid(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return current->gid;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_getegid(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return current->egid;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_nice(long increment)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int newprio;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (increment < 0 && !suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	newprio = current->priority - increment;
// TODO WGJA WIP: 	if (newprio < 1)
// TODO WGJA WIP: 		newprio = 1;
// TODO WGJA WIP: 	if (newprio > 35)
// TODO WGJA WIP: 		newprio = 35;
// TODO WGJA WIP: 	current->priority = newprio;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }

static void show_task(int nr,struct task_struct * p)
{
	int i, j;
	unsigned char * stack;

	printk("%d: pid=%d, state=%d, father=%d, child=%d, ",(p == current)?-nr:nr,p->pid,
		p->state, p->p_pptr->pid, p->p_cptr ? p->p_cptr->pid : -1);
	i = 0;
	j = PAGE_SIZE;
	if (!(stack = (unsigned char *) p->kernel_stack_page)) {
		stack = (unsigned char *) init_kernel_stack;
		j = sizeof(init_kernel_stack);
	}
	while (i<j && !*(stack++))
		i++;
	printk("%d/%d chars free in kstack\n",i,j);
	printk("   PC=%08X.", *(1019 + (unsigned long *) p));
	if (p->p_ysptr || p->p_osptr) 
		printk("   Younger sib=%d, older sib=%d\n", 
			p->p_ysptr ? p->p_ysptr->pid : -1,
			p->p_osptr ? p->p_osptr->pid : -1);
	else
		printk("\n");
}

void show_state(void)
{
	int i;

	printk("Task-info:\n");
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i])
			show_task(i,task[i]);
}

void sched_init(void)
{
	int i;
	struct desc_struct * p;

	if (sizeof(struct sigaction) != 16)
		panic("Struct sigaction MUST be 16 bytes");
	set_tss_desc(gdt+FIRST_TSS_ENTRY,&init_task.tss);
	set_ldt_desc(gdt+FIRST_LDT_ENTRY,&default_ldt,1);
	set_system_gate(0x80,&system_call);
	p = gdt+2+FIRST_TSS_ENTRY;
	for(i=1 ; i<NR_TASKS ; i++) {
		task[i] = (task_struct*) NULL;
		p->a=p->b=0;
		p++;
		p->a=p->b=0;
		p++;
	}
/* Clear NT, so that we won't have troubles with that later on */
	__asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
	load_TR(0);
	load_ldt(0);
	outb_p(0x34,0x43);		/* binary, mode 2, LSB/MSB, ch 0 */
	outb_p(LATCH & 0xff , 0x40);	/* LSB */
	outb(LATCH >> 8 , 0x40);	/* MSB */
	if (request_irq(TIMER_IRQ,(void (*)(int)) do_timer)!=0)
		panic("Could not allocate timer IRQ!");
}
