#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

// TODO WGJA WIP: #define NEW_SWAP
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * define DEBUG if you want the wait-queues to have some extra
// TODO WGJA WIP:  * debugging code. It's not normally used, but might catch some
// TODO WGJA WIP:  * wait-queue coding errors.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  *  #define DEBUG
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define HZ 100
// TODO WGJA WIP: 
// TODO WGJA WIP: #include <linux/tasks.h>
// TODO WGJA WIP: #include <asm/system.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * User space process size: 3GB. This is hardcoded into a few places,
// TODO WGJA WIP:  * so don't change it unless you know what you are doing.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define TASK_SIZE	0xc0000000
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Size of io_bitmap in longwords: 32 is ports 0-0x3ff.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define IO_BITMAP_SIZE	32
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * These are the constant used to fake the fixed-point load-average
// TODO WGJA WIP:  * counting. Some notes:
// TODO WGJA WIP:  *  - 11 bit fractions expand to 22 bits by the multiplies: this gives
// TODO WGJA WIP:  *    a load-average precision of 10 bits integer + 11 bits fractional
// TODO WGJA WIP:  *  - if you want to count load-averages more often, you need more
// TODO WGJA WIP:  *    precision, or rounding will get you. With 2-second counting freq,
// TODO WGJA WIP:  *    the EXP_n values would be 1981, 2034 and 2043 if still using only
// TODO WGJA WIP:  *    11 bit fractions.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern unsigned long avenrun[];		/* Load averages */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define FSHIFT		11		/* nr of bits of precision */
// TODO WGJA WIP: #define FIXED_1		(1<<FSHIFT)	/* 1.0 as fixed-point */
// TODO WGJA WIP: #define LOAD_FREQ	(5*HZ)		/* 5 sec intervals */
// TODO WGJA WIP: #define EXP_1		1884		/* 1/exp(5sec/1min) as fixed-point */
// TODO WGJA WIP: #define EXP_5		2014		/* 1/exp(5sec/5min) */
// TODO WGJA WIP: #define EXP_15		2037		/* 1/exp(5sec/15min) */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define CALC_LOAD(load,exp,n) \
// TODO WGJA WIP: 	load *= exp; \
// TODO WGJA WIP: 	load += n*(FIXED_1-exp); \
// TODO WGJA WIP: 	load >>= FSHIFT;
// TODO WGJA WIP: 
// TODO WGJA WIP: #define CT_TO_SECS(x)	((x) / HZ)
// TODO WGJA WIP: #define CT_TO_USECS(x)	(((x) % HZ) * 1000000/HZ)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define FIRST_TASK task[0]
// TODO WGJA WIP: #define LAST_TASK task[NR_TASKS-1]
// TODO WGJA WIP: 
// TODO WGJA WIP: #include <linux/head.h>
// TODO WGJA WIP: #include <linux/fs.h>
// TODO WGJA WIP: #include <linux/mm.h>
// TODO WGJA WIP: #include <linux/signal.h>
// TODO WGJA WIP: #include <linux/time.h>
// TODO WGJA WIP: #include <linux/param.h>
// TODO WGJA WIP: #include <linux/resource.h>
// TODO WGJA WIP: #include <linux/vm86.h>
// TODO WGJA WIP: #include <linux/math_emu.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: #define TASK_RUNNING		0
// TODO WGJA WIP: #define TASK_INTERRUPTIBLE	1
// TODO WGJA WIP: #define TASK_UNINTERRUPTIBLE	2
// TODO WGJA WIP: #define TASK_ZOMBIE		3
// TODO WGJA WIP: #define TASK_STOPPED		4
// TODO WGJA WIP: #define TASK_SWAPPING		5
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifndef NULL
// TODO WGJA WIP: #define NULL ((void *) 0)
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifdef __KERNEL__
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void sched_init(void);
// TODO WGJA WIP: extern void show_state(void);
// TODO WGJA WIP: extern void trap_init(void);
// TODO WGJA WIP: extern void panic(const char * str);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" void schedule(void);
// TODO WGJA WIP: 
// TODO WGJA WIP: #endif /* __KERNEL__ */
// TODO WGJA WIP: 
// TODO WGJA WIP: struct i387_hard_struct {
// TODO WGJA WIP: 	long	cwd;
// TODO WGJA WIP: 	long	swd;
// TODO WGJA WIP: 	long	twd;
// TODO WGJA WIP: 	long	fip;
// TODO WGJA WIP: 	long	fcs;
// TODO WGJA WIP: 	long	foo;
// TODO WGJA WIP: 	long	fos;
// TODO WGJA WIP: 	long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: struct i387_soft_struct {
// TODO WGJA WIP: 	long	cwd;
// TODO WGJA WIP: 	long	swd;
// TODO WGJA WIP: 	long	twd;
// TODO WGJA WIP: 	long	fip;
// TODO WGJA WIP: 	long	fcs;
// TODO WGJA WIP: 	long	foo;
// TODO WGJA WIP: 	long	fos;
// TODO WGJA WIP: 	long    top;
// TODO WGJA WIP: 	struct fpu_reg	regs[8];	/* 8*16 bytes for each FP-reg = 128 bytes */
// TODO WGJA WIP: 	unsigned char	lookahead;
// TODO WGJA WIP: 	struct info	*info;
// TODO WGJA WIP: 	unsigned long	entry_eip;
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: union i387_union {
// TODO WGJA WIP: 	struct i387_hard_struct hard;
// TODO WGJA WIP: 	struct i387_soft_struct soft;
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: struct tss_struct {
// TODO WGJA WIP: 	unsigned short	back_link,__blh;
// TODO WGJA WIP: 	unsigned long	esp0;
// TODO WGJA WIP: 	unsigned short	ss0,__ss0h;
// TODO WGJA WIP: 	unsigned long	esp1;
// TODO WGJA WIP: 	unsigned short	ss1,__ss1h;
// TODO WGJA WIP: 	unsigned long	esp2;
// TODO WGJA WIP: 	unsigned short	ss2,__ss2h;
// TODO WGJA WIP: 	unsigned long	cr3;
// TODO WGJA WIP: 	unsigned long	eip;
// TODO WGJA WIP: 	unsigned long	eflags;
// TODO WGJA WIP: 	unsigned long	eax,ecx,edx,ebx;
// TODO WGJA WIP: 	unsigned long	esp;
// TODO WGJA WIP: 	unsigned long	ebp;
// TODO WGJA WIP: 	unsigned long	esi;
// TODO WGJA WIP: 	unsigned long	edi;
// TODO WGJA WIP: 	unsigned short	es, __esh;
// TODO WGJA WIP: 	unsigned short	cs, __csh;
// TODO WGJA WIP: 	unsigned short	ss, __ssh;
// TODO WGJA WIP: 	unsigned short	ds, __dsh;
// TODO WGJA WIP: 	unsigned short	fs, __fsh;
// TODO WGJA WIP: 	unsigned short	gs, __gsh;
// TODO WGJA WIP: 	unsigned short	ldt, __ldth;
// TODO WGJA WIP: 	unsigned short	trace, bitmap;
// TODO WGJA WIP: 	unsigned long	io_bitmap[IO_BITMAP_SIZE+1];
// TODO WGJA WIP: 	unsigned long	tr;
// TODO WGJA WIP: 	union i387_union i387;
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: struct task_struct {
// TODO WGJA WIP: /* these are hardcoded - don't touch */
// TODO WGJA WIP: 	long state;		/* -1 unrunnable, 0 runnable, >0 stopped */
// TODO WGJA WIP: 	long counter;
// TODO WGJA WIP: 	long priority;
// TODO WGJA WIP: 	unsigned long signal;
// TODO WGJA WIP: 	unsigned long blocked;	/* bitmap of masked signals */
// TODO WGJA WIP: 	unsigned long flags;	/* per process flags, defined below */
// TODO WGJA WIP: 	int errno;
// TODO WGJA WIP: /* various fields */
// TODO WGJA WIP: 	struct task_struct *next_task, *prev_task;
// TODO WGJA WIP: 	struct sigaction sigaction[32];
// TODO WGJA WIP: 	unsigned long saved_kernel_stack;
// TODO WGJA WIP: 	unsigned long kernel_stack_page;
// TODO WGJA WIP: 	int exit_code, exit_signal;
// TODO WGJA WIP: 	int elf_executable:1;
// TODO WGJA WIP: 	int dumpable:1;
// TODO WGJA WIP: 	int swappable:1;
// TODO WGJA WIP: 	unsigned long start_code,end_code,end_data,start_brk,brk,start_stack,start_mmap;
// TODO WGJA WIP: 	unsigned long arg_start, arg_end, env_start, env_end;
// TODO WGJA WIP: 	long pid,pgrp,session,leader;
// TODO WGJA WIP: 	int	groups[NGROUPS];
// TODO WGJA WIP: 	/* 
// TODO WGJA WIP: 	 * pointers to (original) parent process, youngest child, younger sibling,
// TODO WGJA WIP: 	 * older sibling, respectively.  (p->father can be replaced with 
// TODO WGJA WIP: 	 * p->p_pptr->pid)
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	struct task_struct *p_opptr,*p_pptr, *p_cptr, *p_ysptr, *p_osptr;
// TODO WGJA WIP: 	struct wait_queue *wait_chldexit;	/* for wait4() */
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * For ease of programming... Normal sleeps don't need to
// TODO WGJA WIP: 	 * keep track of a wait-queue: every task has an entry of it's own
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	unsigned short uid,euid,suid;
// TODO WGJA WIP: 	unsigned short gid,egid,sgid;
// TODO WGJA WIP: 	unsigned long timeout;
// TODO WGJA WIP: 	unsigned long it_real_value, it_prof_value, it_virt_value;
// TODO WGJA WIP: 	unsigned long it_real_incr, it_prof_incr, it_virt_incr;
// TODO WGJA WIP: 	long utime,stime,cutime,cstime,start_time;
// TODO WGJA WIP: 	unsigned long min_flt, maj_flt;
// TODO WGJA WIP: 	unsigned long cmin_flt, cmaj_flt;
// TODO WGJA WIP: 	struct rlimit rlim[RLIM_NLIMITS]; 
// TODO WGJA WIP: 	unsigned short used_math;
// TODO WGJA WIP: 	unsigned short rss;	/* number of resident pages */
// TODO WGJA WIP: 	char comm[16];
// TODO WGJA WIP: 	struct vm86_struct * vm86_info;
// TODO WGJA WIP: 	unsigned long screen_bitmap;
// TODO WGJA WIP: /* file system info */
// TODO WGJA WIP: 	int link_count;
// TODO WGJA WIP: 	int tty;		/* -1 if no tty, so it must be signed */
// TODO WGJA WIP: 	unsigned short umask;
// TODO WGJA WIP: 	struct inode * pwd;
// TODO WGJA WIP: 	struct inode * root;
// TODO WGJA WIP: 	struct inode * executable;
// TODO WGJA WIP: 	struct vm_area_struct * mmap;
// TODO WGJA WIP: 	struct shm_desc *shm;
// TODO WGJA WIP: 	struct sem_undo *semun;
// TODO WGJA WIP: 	struct file * filp[NR_OPEN];
// TODO WGJA WIP: 	fd_set close_on_exec;
// TODO WGJA WIP: /* ldt for this task - used by Wine.  If NULL, default_ldt is used */
// TODO WGJA WIP: 	struct desc_struct *ldt;
// TODO WGJA WIP: /* tss for this task */
// TODO WGJA WIP: 	struct tss_struct tss;
// TODO WGJA WIP: #ifdef NEW_SWAP
// TODO WGJA WIP: 	unsigned long old_maj_flt;	/* old value of maj_flt */
// TODO WGJA WIP: 	unsigned long dec_flt;		/* page fault count of the last time */
// TODO WGJA WIP: 	unsigned long swap_cnt;		/* number of pages to swap on next pass */
// TODO WGJA WIP: 	short swap_table;		/* current page table */
// TODO WGJA WIP: 	short swap_page;		/* current page */
// TODO WGJA WIP: #endif NEW_SWAP
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Per process flags
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define PF_ALIGNWARN	0x00000001	/* Print alignment warning msgs */
// TODO WGJA WIP: 					/* Not implemented yet, only for 486*/
// TODO WGJA WIP: #define PF_PTRACED	0x00000010	/* set if ptrace (0) has been called. */
// TODO WGJA WIP: #define PF_TRACESYS	0x00000020	/* tracing system calls */
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * cloning flags:
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define CSIGNAL		0x000000ff	/* signal mask to be sent at exit */
// TODO WGJA WIP: #define COPYVM		0x00000100	/* set if VM copy desired (like normal fork()) */
// TODO WGJA WIP: #define COPYFD		0x00000200	/* set if fd's should be copied, not shared (NI) */
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  *  INIT_TASK is used to set up the first task table, touch at
// TODO WGJA WIP:  * your own risk!. Base=0, limit=0x1fffff (=2MB)
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define INIT_TASK \
// TODO WGJA WIP: /* state etc */	{ 0,15,15,0,0,0,0, \
// TODO WGJA WIP: /* schedlink */	&init_task,&init_task, \
// TODO WGJA WIP: /* signals */	{{ 0, },}, \
// TODO WGJA WIP: /* stack */	0,0, \
// TODO WGJA WIP: /* ec,brk... */	0,0,0,0,0,0,0,0,0,0,0,0, \
// TODO WGJA WIP: /* argv.. */	0,0,0,0, \
// TODO WGJA WIP: /* pid etc.. */	0,0,0,0, \
// TODO WGJA WIP: /* suppl grps*/ {NOGROUP,}, \
// TODO WGJA WIP: /* proc links*/ &init_task,&init_task,NULL,NULL,NULL,NULL, \
// TODO WGJA WIP: /* uid etc */	0,0,0,0,0,0, \
// TODO WGJA WIP: /* timeout */	0,0,0,0,0,0,0,0,0,0,0,0, \
// TODO WGJA WIP: /* min_flt */	0,0,0,0, \
// TODO WGJA WIP: /* rlimits */   { {LONG_MAX, LONG_MAX}, {LONG_MAX, LONG_MAX},  \
// TODO WGJA WIP: 		  {LONG_MAX, LONG_MAX}, {LONG_MAX, LONG_MAX},  \
// TODO WGJA WIP: 		  {LONG_MAX, LONG_MAX}, {LONG_MAX, LONG_MAX}}, \
// TODO WGJA WIP: /* math */	0, \
// TODO WGJA WIP: /* rss */	2, \
// TODO WGJA WIP: /* comm */	"swapper", \
// TODO WGJA WIP: /* vm86_info */	NULL, 0, \
// TODO WGJA WIP: /* fs info */	0,-1,0022,NULL,NULL,NULL,NULL, \
// TODO WGJA WIP: /* ipc */	NULL, NULL, \
// TODO WGJA WIP: /* filp */	{NULL,}, \
// TODO WGJA WIP: /* cloe */	{{ 0, }}, \
// TODO WGJA WIP: /* ldt */	NULL, \
// TODO WGJA WIP: /*tss*/	{0,0, \
// TODO WGJA WIP: 	 sizeof(init_kernel_stack) + (long) &init_kernel_stack, KERNEL_DS, 0, \
// TODO WGJA WIP: 	 0,0,0,0,0,0, \
// TODO WGJA WIP: 	 (long) &swapper_pg_dir, \
// TODO WGJA WIP: 	 0,0,0,0,0,0,0,0,0,0, \
// TODO WGJA WIP: 	 USER_DS,0,USER_DS,0,USER_DS,0,USER_DS,0,USER_DS,0,USER_DS,0, \
// TODO WGJA WIP: 	 _LDT(0),0, \
// TODO WGJA WIP: 	 0, 0x8000, \
// TODO WGJA WIP: /* ioperm */ 	{~0, }, \
// TODO WGJA WIP: 	 _TSS(0), \
// TODO WGJA WIP: /* 387 state */	{ { 0, }, } \
// TODO WGJA WIP: 	} \
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern struct task_struct init_task;
// TODO WGJA WIP: extern struct task_struct *task[NR_TASKS];
// TODO WGJA WIP: extern struct task_struct *last_task_used_math;
// TODO WGJA WIP: extern struct task_struct *current;
// TODO WGJA WIP: extern unsigned long volatile jiffies;
// TODO WGJA WIP: extern unsigned long startup_time;
// TODO WGJA WIP: extern int jiffies_offset;
// TODO WGJA WIP: extern int need_resched;
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int hard_math;
// TODO WGJA WIP: extern int x86;
// TODO WGJA WIP: extern int ignore_irq13;
// TODO WGJA WIP: extern int wp_works_ok;
// TODO WGJA WIP: 
// TODO WGJA WIP: #define CURRENT_TIME (startup_time+(jiffies+jiffies_offset)/HZ)
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void sleep_on(struct wait_queue ** p);
// TODO WGJA WIP: extern void interruptible_sleep_on(struct wait_queue ** p);
// TODO WGJA WIP: extern void wake_up(struct wait_queue ** p);
// TODO WGJA WIP: extern void wake_up_interruptible(struct wait_queue ** p);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void notify_parent(struct task_struct * tsk);
// TODO WGJA WIP: extern int send_sig(unsigned long sig,struct task_struct * p,int priv);
// TODO WGJA WIP: extern int in_group_p(gid_t grp);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int request_irq(unsigned int irq,void (*handler)(int));
// TODO WGJA WIP: extern void free_irq(unsigned int irq);
// TODO WGJA WIP: extern int irqaction(unsigned int irq,struct sigaction * sa);
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Entry into gdt where to find first TSS. GDT layout:
// TODO WGJA WIP:  *   0 - nul
// TODO WGJA WIP:  *   1 - kernel code segment
// TODO WGJA WIP:  *   2 - kernel data segment
// TODO WGJA WIP:  *   3 - user code segment
// TODO WGJA WIP:  *   4 - user data segment
// TODO WGJA WIP:  * ...
// TODO WGJA WIP:  *   8 - TSS #0
// TODO WGJA WIP:  *   9 - LDT #0
// TODO WGJA WIP:  *  10 - TSS #1
// TODO WGJA WIP:  *  11 - LDT #1
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define FIRST_TSS_ENTRY 8
// TODO WGJA WIP: #define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
// TODO WGJA WIP: #define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
// TODO WGJA WIP: #define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))
// TODO WGJA WIP: #define load_TR(n) __asm__("ltr %%ax": /* no output */ :"a" (_TSS(n)))
// TODO WGJA WIP: #define load_ldt(n) __asm__("lldt %%ax": /* no output */ :"a" (_LDT(n)))
// TODO WGJA WIP: #define store_TR(n) \
// TODO WGJA WIP: __asm__("str %%ax\n\t" \
// TODO WGJA WIP: 	"subl %2,%%eax\n\t" \
// TODO WGJA WIP: 	"shrl $4,%%eax" \
// TODO WGJA WIP: 	:"=a" (n) \
// TODO WGJA WIP: 	:"0" (0),"i" (FIRST_TSS_ENTRY<<3))
// TODO WGJA WIP: /*
// TODO WGJA WIP:  *	switch_to(n) should switch tasks to task nr n, first
// TODO WGJA WIP:  * checking that n isn't the current task, in which case it does nothing.
// TODO WGJA WIP:  * This also clears the TS-flag if the task we switched to has used
// TODO WGJA WIP:  * tha math co-processor latest.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define switch_to(tsk) \
// TODO WGJA WIP: __asm__("cmpl %%ecx,_current\n\t" \
// TODO WGJA WIP: 	"je 1f\n\t" \
// TODO WGJA WIP: 	"cli\n\t" \
// TODO WGJA WIP: 	"xchgl %%ecx,_current\n\t" \
// TODO WGJA WIP: 	"ljmp %0\n\t" \
// TODO WGJA WIP: 	"sti\n\t" \
// TODO WGJA WIP: 	"cmpl %%ecx,_last_task_used_math\n\t" \
// TODO WGJA WIP: 	"jne 1f\n\t" \
// TODO WGJA WIP: 	"clts\n" \
// TODO WGJA WIP: 	"1:" \
// TODO WGJA WIP: 	: /* no output */ \
// TODO WGJA WIP: 	:"m" (*(((char *)&tsk->tss.tr)-4)), \
// TODO WGJA WIP: 	 "c" (tsk) \
// TODO WGJA WIP: 	:"cx")
// TODO WGJA WIP: 
// TODO WGJA WIP: #define _set_base(addr,base) \
// TODO WGJA WIP: __asm__("movw %%dx,%0\n\t" \
// TODO WGJA WIP: 	"rorl $16,%%edx\n\t" \
// TODO WGJA WIP: 	"movb %%dl,%1\n\t" \
// TODO WGJA WIP: 	"movb %%dh,%2" \
// TODO WGJA WIP: 	: /* no output */ \
// TODO WGJA WIP: 	:"m" (*((addr)+2)), \
// TODO WGJA WIP: 	 "m" (*((addr)+4)), \
// TODO WGJA WIP: 	 "m" (*((addr)+7)), \
// TODO WGJA WIP: 	 "d" (base) \
// TODO WGJA WIP: 	:"dx")
// TODO WGJA WIP: 
// TODO WGJA WIP: #define _set_limit(addr,limit) \
// TODO WGJA WIP: __asm__("movw %%dx,%0\n\t" \
// TODO WGJA WIP: 	"rorl $16,%%edx\n\t" \
// TODO WGJA WIP: 	"movb %1,%%dh\n\t" \
// TODO WGJA WIP: 	"andb $0xf0,%%dh\n\t" \
// TODO WGJA WIP: 	"orb %%dh,%%dl\n\t" \
// TODO WGJA WIP: 	"movb %%dl,%1" \
// TODO WGJA WIP: 	: /* no output */ \
// TODO WGJA WIP: 	:"m" (*(addr)), \
// TODO WGJA WIP: 	 "m" (*((addr)+6)), \
// TODO WGJA WIP: 	 "d" (limit) \
// TODO WGJA WIP: 	:"dx")
// TODO WGJA WIP: 
// TODO WGJA WIP: #define set_base(ldt,base) _set_base( ((char *)&(ldt)) , base )
// TODO WGJA WIP: #define set_limit(ldt,limit) _set_limit( ((char *)&(ldt)) , (limit-1)>>12 )
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * The wait-queues are circular lists, and you have to be *very* sure
// TODO WGJA WIP:  * to keep them correct. Use only these two functions to add/remove
// TODO WGJA WIP:  * entries in the queues.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern inline void add_wait_queue(struct wait_queue ** p, struct wait_queue * wait)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifdef DEBUG
// TODO WGJA WIP: 	if (wait->next) {
// TODO WGJA WIP: 		unsigned long pc;
// TODO WGJA WIP: 		__asm__ __volatile__("call 1f\n"
// TODO WGJA WIP: 			"1:\tpopl %0":"=r" (pc));
// TODO WGJA WIP: 		printk("add_wait_queue (%08x): wait->next = %08x\n",pc,wait->next);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	if (!*p) {
// TODO WGJA WIP: 		wait->next = wait;
// TODO WGJA WIP: 		*p = wait;
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		wait->next = (*p)->next;
// TODO WGJA WIP: 		(*p)->next = wait;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline void remove_wait_queue(struct wait_queue ** p, struct wait_queue * wait)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 	struct wait_queue * tmp;
// TODO WGJA WIP: #ifdef DEBUG
// TODO WGJA WIP: 	unsigned long ok = 0;
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	if ((*p == wait) &&
// TODO WGJA WIP: #ifdef DEBUG
// TODO WGJA WIP: 	    (ok = 1) &&
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	    ((*p = wait->next) == wait)) {
// TODO WGJA WIP: 		*p = (wait_queue *)NULL;
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		tmp = wait;
// TODO WGJA WIP: 		while (tmp->next != wait) {
// TODO WGJA WIP: 			tmp = tmp->next;
// TODO WGJA WIP: #ifdef DEBUG
// TODO WGJA WIP: 			if (tmp == *p)
// TODO WGJA WIP: 				ok = 1;
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		tmp->next = wait->next;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	wait->next = (wait_queue *)NULL;
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: #ifdef DEBUG
// TODO WGJA WIP: 	if (!ok) {
// TODO WGJA WIP: 		printk("removed wait_queue not on list.\n");
// TODO WGJA WIP: 		printk("list = %08x, queue = %08x\n",p,wait);
// TODO WGJA WIP: 		__asm__("call 1f\n1:\tpopl %0":"=r" (ok));
// TODO WGJA WIP: 		printk("eip = %08x\n",ok);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: #endif
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern inline void select_wait(struct wait_queue ** wait_address, select_table * p)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct select_table_entry * entry;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!p || !wait_address)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (p->nr >= __MAX_SELECT_TABLE_ENTRIES)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP:  	entry = p->entry + p->nr;
// TODO WGJA WIP: 	entry->wait_address = wait_address;
// TODO WGJA WIP: 	entry->wait.task = current;
// TODO WGJA WIP: 	entry->wait.next = (wait_queue *)NULL;
// TODO WGJA WIP: 	add_wait_queue(wait_address,&entry->wait);
// TODO WGJA WIP: 	p->nr++;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static inline unsigned long _get_base(char * addr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long __base;
// TODO WGJA WIP: 	__asm__("movb %3,%%dh\n\t"
// TODO WGJA WIP: 		"movb %2,%%dl\n\t"
// TODO WGJA WIP: 		"shll $16,%%edx\n\t"
// TODO WGJA WIP: 		"movw %1,%%dx"
// TODO WGJA WIP: 		:"=&d" (__base)
// TODO WGJA WIP: 		:"m" (*((addr)+2)),
// TODO WGJA WIP: 		 "m" (*((addr)+4)),
// TODO WGJA WIP: 		 "m" (*((addr)+7)));
// TODO WGJA WIP: 	return __base;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: #define get_base(ldt) _get_base( ((char *)&(ldt)) )
// TODO WGJA WIP: 
// TODO WGJA WIP: static inline unsigned long get_limit(unsigned long segment)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long __limit;
// TODO WGJA WIP: 	__asm__("lsll %1,%0"
// TODO WGJA WIP: 		:"=r" (__limit):"r" (segment));
// TODO WGJA WIP: 	return __limit+1;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: #define REMOVE_LINKS(p) do { unsigned long flags; \
// TODO WGJA WIP: 	save_flags(flags) ; cli(); \
// TODO WGJA WIP: 	(p)->next_task->prev_task = (p)->prev_task; \
// TODO WGJA WIP: 	(p)->prev_task->next_task = (p)->next_task; \
// TODO WGJA WIP: 	restore_flags(flags); \
// TODO WGJA WIP: 	if ((p)->p_osptr) \
// TODO WGJA WIP: 		(p)->p_osptr->p_ysptr = (p)->p_ysptr; \
// TODO WGJA WIP: 	if ((p)->p_ysptr) \
// TODO WGJA WIP: 		(p)->p_ysptr->p_osptr = (p)->p_osptr; \
// TODO WGJA WIP: 	else \
// TODO WGJA WIP: 		(p)->p_pptr->p_cptr = (p)->p_osptr; \
// TODO WGJA WIP: 	} while (0)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define SET_LINKS(p) do { unsigned long flags; \
// TODO WGJA WIP: 	save_flags(flags); cli(); \
// TODO WGJA WIP: 	(p)->next_task = &init_task; \
// TODO WGJA WIP: 	(p)->prev_task = init_task.prev_task; \
// TODO WGJA WIP: 	init_task.prev_task->next_task = (p); \
// TODO WGJA WIP: 	init_task.prev_task = (p); \
// TODO WGJA WIP: 	restore_flags(flags); \
// TODO WGJA WIP: 	(p)->p_ysptr = NULL; \
// TODO WGJA WIP: 	if (((p)->p_osptr = (p)->p_pptr->p_cptr) != NULL) \
// TODO WGJA WIP: 		(p)->p_osptr->p_ysptr = p; \
// TODO WGJA WIP: 	(p)->p_pptr->p_cptr = p; \
// TODO WGJA WIP: 	} while (0)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define for_each_task(p) \
// TODO WGJA WIP: 	for (p = &init_task ; (p = p->next_task) != &init_task ; )
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This is the ldt that every process will get unless we need
// TODO WGJA WIP:  * something other than this.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern struct desc_struct default_ldt;

#endif
