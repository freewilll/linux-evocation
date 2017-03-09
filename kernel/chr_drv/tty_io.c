/*
 *  linux/kernel/tty_io.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * 'tty_io.c' gives an orthogonal feeling to tty's, be they consoles
 * or rs-channels. It also implements echoing, cooked mode etc.
 *
 * Kill-line thanks to John T Kohl, who also corrected VMIN = VTIME = 0.
 *
 * Modified by Theodore Ts'o, 9/14/92, to dynamically allocate the
 * tty_struct and tty_queue structures.  Previously there was a array
 * of 256 tty_struct's which was statically allocated, and the
 * tty_queue structures were allocated at boot time.  Both are now
 * dynamically allocated only when the tty is open.
 *
 * Also restructured routines so that there is more of a separation
 * between the high-level tty routines (tty_io.c and tty_ioctl.c) and
 * the low-level tty routines (serial.c, pty.c, console.c).  This
 * makes for cleaner and more compact code.  -TYT, 9/17/92 
 *
 * Modified by Fred N. van Kempen, 01/29/93, to add line disciplines
 * which can be dynamically activated and de-activated by the line
 * discipline handling modules (like SLIP).
 *
 * NOTE: pay no attention to the line discpline code (yet); its
 * interface is still subject to change in this version...
 * -- TYT, 1/31/92
 *
 * Added functionality to the OPOST tty handling.  No delays, but all
 * other bits should be there.
 *	-- Nick Holloway <alfie@dcs.warwick.ac.uk>, 27th May 1993.
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/interrupt.h>
// TODO WGJA WIP: #include <linux/kd.h>
#include <linux/mm.h>
#include <linux/string.h>
// TODO WGJA WIP: #include <linux/keyboard.h>

#include <asm/segment.h>
#include <asm/system.h>
#include <asm/bitops.h>

#include "vt_kern.h"

#define MAX_TTYS 256

struct tty_struct *tty_table[MAX_TTYS];
 struct termios *tty_termios[MAX_TTYS];	/* We need to keep the termios state */
				  	/* around, even when a tty is closed */
struct termios *termios_locked[MAX_TTYS]; /* Bitfield of locked termios flags*/
struct tty_ldisc ldiscs[NR_LDISCS];	/* line disc dispatch table	*/
int tty_check_write[MAX_TTYS/32];	/* bitfield for the bh handler */

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * fg_console is the current virtual console,
// TODO WGJA WIP:  * redirect is the pseudo-tty that console output
// TODO WGJA WIP:  * is redirected to if asked by TIOCCONS.
// TODO WGJA WIP:  */
// TODO WGJA WIP: int fg_console = 0;
// TODO WGJA WIP: struct tty_struct * redirect = NULL;
// TODO WGJA WIP: struct wait_queue * keypress_wait = NULL;
// TODO WGJA WIP: 
// TODO WGJA WIP: static void initialize_tty_struct(int line, struct tty_struct *tty);
// TODO WGJA WIP: static void initialize_termios(int line, struct termios *tp);
// TODO WGJA WIP: 
// TODO WGJA WIP: static int tty_read(struct inode *, struct file *, char *, int);
// TODO WGJA WIP: static int tty_write(struct inode *, struct file *, char *, int);
// TODO WGJA WIP: static int tty_select(struct inode *, struct file *, int, select_table *);
// TODO WGJA WIP: static int tty_open(struct inode *, struct file *);
// TODO WGJA WIP: static void tty_release(struct inode *, struct file *);

int tty_register_ldisc(int disc, struct tty_ldisc *new_ldisc)
{
	if (disc < N_TTY || disc >= NR_LDISCS)
		return -EINVAL;

	if (new_ldisc) {
		ldiscs[disc] = *new_ldisc;
		ldiscs[disc].flags |= LDISC_FLAG_DEFINED;
	} else
		memset(&ldiscs[disc], 0, sizeof(struct tty_ldisc));

	return 0;
}

// TODO WGJA WIP: void put_tty_queue(char c, struct tty_queue * queue)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int head;
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	head = (queue->head + 1) & (TTY_BUF_SIZE-1);
// TODO WGJA WIP: 	if (head != queue->tail) {
// TODO WGJA WIP: 		queue->buf[queue->head] = c;
// TODO WGJA WIP: 		queue->head = head;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int get_tty_queue(struct tty_queue * queue)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int result = -1;
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	if (queue->tail != queue->head) {
// TODO WGJA WIP: 		result = 0xff & queue->buf[queue->tail];
// TODO WGJA WIP: 		queue->tail = (queue->tail + 1) & (TTY_BUF_SIZE-1);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: 	return result;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This routine copies out a maximum of buflen characters from the
// TODO WGJA WIP:  * read_q; it is a convenience for line disciplines so they can grab a
// TODO WGJA WIP:  * large block of data without calling get_tty_char directly.  It
// TODO WGJA WIP:  * returns the number of characters actually read. Return terminates
// TODO WGJA WIP:  * if an error character is read from the queue and the return value
// TODO WGJA WIP:  * is negated.
// TODO WGJA WIP:  */
// TODO WGJA WIP: int tty_read_raw_data(struct tty_struct *tty, unsigned char *bufp, int buflen)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int	result = 0;
// TODO WGJA WIP: 	unsigned char	*p = bufp;
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 	int head, tail;
// TODO WGJA WIP: 	int ok = 1;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	tail = tty->read_q.tail;
// TODO WGJA WIP: 	head = tty->read_q.head;
// TODO WGJA WIP: 	while ((result < buflen) && (tail!=head) && ok) {
// TODO WGJA WIP: 		ok = !clear_bit (tail, &tty->readq_flags);
// TODO WGJA WIP: 		*p++ =  tty->read_q.buf[tail++];
// TODO WGJA WIP: 		tail &= TTY_BUF_SIZE-1;
// TODO WGJA WIP: 		result++;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	tty->read_q.tail = tail;
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: 	return (ok) ? result : -result;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: 
// TODO WGJA WIP: void tty_write_flush(struct tty_struct * tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (!tty->write || EMPTY(&tty->write_q))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (set_bit(TTY_WRITE_BUSY,&tty->flags))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	tty->write(tty);
// TODO WGJA WIP: 	if (!clear_bit(TTY_WRITE_BUSY,&tty->flags))
// TODO WGJA WIP: 		printk("tty_write_flush: bit already cleared\n");
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void tty_read_flush(struct tty_struct * tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (!tty || EMPTY(&tty->read_q))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (set_bit(TTY_READ_BUSY, &tty->flags))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	ldiscs[tty->disc].handler(tty);
// TODO WGJA WIP: 	if (!clear_bit(TTY_READ_BUSY, &tty->flags))
// TODO WGJA WIP: 		printk("tty_read_flush: bit already cleared\n");
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int hung_up_tty_read(struct inode * inode, struct file * file, char * buf, int count)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int hung_up_tty_write(struct inode * inode, struct file * file, char * buf, int count)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -EIO;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int hung_up_tty_select(struct inode * inode, struct file * filp, int sel_type, select_table * wait)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int hung_up_tty_ioctl(struct inode * inode, struct file * file,
// TODO WGJA WIP: 			     unsigned int cmd, unsigned long arg)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -EIO;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int tty_lseek(struct inode * inode, struct file * file, off_t offset, int orig)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ESPIPE;
// TODO WGJA WIP: }

static struct file_operations tty_fops = {
	NULL, // TODO WGJA tty fops: tty_lseek,
	NULL, // TODO WGJA tty fops: tty_read,
	NULL, // TODO WGJA tty fops: tty_write,
	NULL, // TODO WGJA tty fops: NULL,		/* tty_readdir */
	NULL, // TODO WGJA tty fops: tty_select,
	NULL, // TODO WGJA tty fops: tty_ioctl,
	NULL, // TODO WGJA tty fops: NULL,		/* tty_mmap */
	NULL, // TODO WGJA tty fops: tty_open,
	NULL, // TODO WGJA tty fops: tty_release
};

// TODO WGJA WIP: static struct file_operations hung_up_tty_fops = {
// TODO WGJA WIP: 	tty_lseek,
// TODO WGJA WIP: 	hung_up_tty_read,
// TODO WGJA WIP: 	hung_up_tty_write,
// TODO WGJA WIP: 	NULL,		/* hung_up_tty_readdir */
// TODO WGJA WIP: 	hung_up_tty_select,
// TODO WGJA WIP: 	tty_ioctl,
// TODO WGJA WIP: 	NULL,		/* hung_up_tty_mmap */
// TODO WGJA WIP: 	tty_open,
// TODO WGJA WIP: 	tty_release
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: static struct file_operations vhung_up_tty_fops = {
// TODO WGJA WIP: 	tty_lseek,
// TODO WGJA WIP: 	hung_up_tty_read,
// TODO WGJA WIP: 	hung_up_tty_write,
// TODO WGJA WIP: 	NULL,		/* hung_up_tty_readdir */
// TODO WGJA WIP: 	hung_up_tty_select,
// TODO WGJA WIP: 	hung_up_tty_ioctl,
// TODO WGJA WIP: 	NULL,		/* hung_up_tty_mmap */
// TODO WGJA WIP: 	tty_open,
// TODO WGJA WIP: 	tty_release
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: void do_tty_hangup(struct tty_struct * tty, struct file_operations *fops)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 	struct file * filp;
// TODO WGJA WIP: 	struct task_struct **p;
// TODO WGJA WIP: 	int dev;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!tty)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	dev = 0x0400 + tty->line;
// TODO WGJA WIP: 	for (filp = first_file, i=0; i<nr_files; i++, filp = filp->f_next) {
// TODO WGJA WIP: 		if (!filp->f_count)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (filp->f_rdev != dev)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (filp->f_inode && filp->f_inode->i_rdev == 0x0400)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (filp->f_op != &tty_fops)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		filp->f_op = fops;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	wake_up_interruptible(&tty->secondary.proc_list);
// TODO WGJA WIP: 	wake_up_interruptible(&tty->read_q.proc_list);
// TODO WGJA WIP: 	wake_up_interruptible(&tty->write_q.proc_list);
// TODO WGJA WIP: 	if (tty->session > 0)
// TODO WGJA WIP: 		kill_sl(tty->session,SIGHUP,1);
// TODO WGJA WIP: 	tty->session = 0;
// TODO WGJA WIP: 	tty->pgrp = -1;
// TODO WGJA WIP:  	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
// TODO WGJA WIP: 		if ((*p) && (*p)->tty == tty->line)
// TODO WGJA WIP: 			(*p)->tty = -1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void tty_hangup(struct tty_struct * tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	do_tty_hangup(tty, &hung_up_tty_fops);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void tty_vhangup(struct tty_struct * tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	do_tty_hangup(tty, &vhung_up_tty_fops);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void tty_unhangup(struct file *filp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	filp->f_op = &tty_fops;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int tty_hung_up_p(struct file * filp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return ((filp->f_op == &hung_up_tty_fops) ||
// TODO WGJA WIP: 		(filp->f_op == &vhung_up_tty_fops));
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Sometimes we want to wait until a particular VT has been activated. We
// TODO WGJA WIP:  * do it in a very simple manner. Everybody waits on a single queue and
// TODO WGJA WIP:  * get woken up at once. Those that are satisfied go on with their business,
// TODO WGJA WIP:  * while those not ready go back to sleep. Seems overkill to add a wait
// TODO WGJA WIP:  * to each vt just for this - usually this does nothing!
// TODO WGJA WIP:  */
// TODO WGJA WIP: static struct wait_queue *vt_activate_queue = NULL;
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Sleeps until a vt is activated, or the task is interrupted. Returns
// TODO WGJA WIP:  * 0 if activation, -1 if interrupted.
// TODO WGJA WIP:  */
// TODO WGJA WIP: int vt_waitactive(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	interruptible_sleep_on(&vt_activate_queue);
// TODO WGJA WIP: 	return (current->signal & ~current->blocked) ? -1 : 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: #define vt_wake_waitactive() wake_up(&vt_activate_queue)
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int kill_proc(int pid, int sig, int priv);
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Performs the back end of a vt switch
// TODO WGJA WIP:  */
// TODO WGJA WIP: void complete_change_console(unsigned int new_console)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned char old_vc_mode;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (new_console == fg_console || new_console >= NR_CONSOLES)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * If we're switching, we could be going from KD_GRAPHICS to
// TODO WGJA WIP: 	 * KD_TEXT mode or vice versa, which means we need to blank or
// TODO WGJA WIP: 	 * unblank the screen later.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	old_vc_mode = vt_cons[fg_console].vc_mode;
// TODO WGJA WIP: 	update_screen(new_console);
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * If this new console is under process control, send it a signal
// TODO WGJA WIP: 	 * telling it that it has acquired. Also check if it has died and
// TODO WGJA WIP: 	 * clean up (similar to logic employed in change_console())
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if (vt_cons[new_console].vt_mode.mode == VT_PROCESS)
// TODO WGJA WIP: 	{
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * Send the signal as privileged - kill_proc() will
// TODO WGJA WIP: 		 * tell us if the process has gone or something else
// TODO WGJA WIP: 		 * is awry
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		if (kill_proc(vt_cons[new_console].vt_pid,
// TODO WGJA WIP: 			      vt_cons[new_console].vt_mode.acqsig,
// TODO WGJA WIP: 			      1) != 0)
// TODO WGJA WIP: 		{
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * The controlling process has died, so we revert back to
// TODO WGJA WIP: 		 * normal operation. In this case, we'll also change back
// TODO WGJA WIP: 		 * to KD_TEXT mode. I'm not sure if this is strictly correct
// TODO WGJA WIP: 		 * but it saves the agony when the X server dies and the screen
// TODO WGJA WIP: 		 * remains blanked due to KD_GRAPHICS! It would be nice to do
// TODO WGJA WIP: 		 * this outside of VT_PROCESS but there is no single process
// TODO WGJA WIP: 		 * to account for and tracking tty count may be undesirable.
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 			vt_cons[new_console].vc_mode = KD_TEXT;
// TODO WGJA WIP: 			clr_vc_kbd_flag(kbd_table + new_console, VC_RAW);
// TODO WGJA WIP:  			vt_cons[new_console].vt_mode.mode = VT_AUTO;
// TODO WGJA WIP:  			vt_cons[new_console].vt_mode.waitv = 0;
// TODO WGJA WIP:  			vt_cons[new_console].vt_mode.relsig = 0;
// TODO WGJA WIP: 			vt_cons[new_console].vt_mode.acqsig = 0;
// TODO WGJA WIP: 			vt_cons[new_console].vt_mode.frsig = 0;
// TODO WGJA WIP: 			vt_cons[new_console].vt_pid = -1;
// TODO WGJA WIP: 			vt_cons[new_console].vt_newvt = -1;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * We do this here because the controlling process above may have
// TODO WGJA WIP: 	 * gone, and so there is now a new vc_mode
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if (old_vc_mode != vt_cons[new_console].vc_mode)
// TODO WGJA WIP: 	{
// TODO WGJA WIP: 		if (vt_cons[new_console].vc_mode == KD_TEXT)
// TODO WGJA WIP: 			unblank_screen();
// TODO WGJA WIP: 		else {
// TODO WGJA WIP: 			timer_active &= ~(1<<BLANK_TIMER);
// TODO WGJA WIP: 			blank_screen();
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Wake anyone waiting for their VT to activate
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	vt_wake_waitactive();
// TODO WGJA WIP: 	return;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Performs the front-end of a vt switch
// TODO WGJA WIP:  */
// TODO WGJA WIP: void change_console(unsigned int new_console)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (new_console == fg_console || new_console >= NR_CONSOLES)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * If this vt is in process mode, then we need to handshake with
// TODO WGJA WIP: 	 * that process before switching. Essentially, we store where that
// TODO WGJA WIP: 	 * vt wants to switch to and wait for it to tell us when it's done
// TODO WGJA WIP: 	 * (via VT_RELDISP ioctl).
// TODO WGJA WIP: 	 *
// TODO WGJA WIP: 	 * We also check to see if the controlling process still exists.
// TODO WGJA WIP: 	 * If it doesn't, we reset this vt to auto mode and continue.
// TODO WGJA WIP: 	 * This is a cheap way to track process control. The worst thing
// TODO WGJA WIP: 	 * that can happen is: we send a signal to a process, it dies, and
// TODO WGJA WIP: 	 * the switch gets "lost" waiting for a response; hopefully, the
// TODO WGJA WIP: 	 * user will try again, we'll detect the process is gone (unless
// TODO WGJA WIP: 	 * the user waits just the right amount of time :-) and revert the
// TODO WGJA WIP: 	 * vt to auto control.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if (vt_cons[fg_console].vt_mode.mode == VT_PROCESS)
// TODO WGJA WIP: 	{
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * Send the signal as privileged - kill_proc() will
// TODO WGJA WIP: 		 * tell us if the process has gone or something else
// TODO WGJA WIP: 		 * is awry
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		if (kill_proc(vt_cons[fg_console].vt_pid,
// TODO WGJA WIP: 			      vt_cons[fg_console].vt_mode.relsig,
// TODO WGJA WIP: 			      1) == 0)
// TODO WGJA WIP: 		{
// TODO WGJA WIP: 			/*
// TODO WGJA WIP: 			 * It worked. Mark the vt to switch to and
// TODO WGJA WIP: 			 * return. The process needs to send us a
// TODO WGJA WIP: 			 * VT_RELDISP ioctl to complete the switch.
// TODO WGJA WIP: 			 */
// TODO WGJA WIP: 			vt_cons[fg_console].vt_newvt = new_console;
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * The controlling process has died, so we revert back to
// TODO WGJA WIP: 		 * normal operation. In this case, we'll also change back
// TODO WGJA WIP: 		 * to KD_TEXT mode. I'm not sure if this is strictly correct
// TODO WGJA WIP: 		 * but it saves the agony when the X server dies and the screen
// TODO WGJA WIP: 		 * remains blanked due to KD_GRAPHICS! It would be nice to do
// TODO WGJA WIP: 		 * this outside of VT_PROCESS but there is no single process
// TODO WGJA WIP: 		 * to account for and tracking tty count may be undesirable.
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		vt_cons[fg_console].vc_mode = KD_TEXT;
// TODO WGJA WIP: 		clr_vc_kbd_flag(kbd_table + fg_console, VC_RAW);
// TODO WGJA WIP: 		vt_cons[fg_console].vt_mode.mode = VT_AUTO;
// TODO WGJA WIP: 		vt_cons[fg_console].vt_mode.waitv = 0;
// TODO WGJA WIP: 		vt_cons[fg_console].vt_mode.relsig = 0;
// TODO WGJA WIP: 		vt_cons[fg_console].vt_mode.acqsig = 0;
// TODO WGJA WIP: 		vt_cons[fg_console].vt_mode.frsig = 0;
// TODO WGJA WIP: 		vt_cons[fg_console].vt_pid = -1;
// TODO WGJA WIP: 		vt_cons[fg_console].vt_newvt = -1;
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * Fall through to normal (VT_AUTO) handling of the switch...
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Ignore all switches in KD_GRAPHICS+VT_AUTO mode
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if (vt_cons[fg_console].vc_mode == KD_GRAPHICS)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	complete_change_console(new_console);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void wait_for_keypress(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	sleep_on(&keypress_wait);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void copy_to_cooked(struct tty_struct * tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int c, special_flag;
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!tty) {
// TODO WGJA WIP: 		printk("copy_to_cooked: called with NULL tty\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!tty->write) {
// TODO WGJA WIP: 		printk("copy_to_cooked: tty %d has null write routine\n",
// TODO WGJA WIP: 		       tty->line);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	while (1) {
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * Check to see how much room we have left in the
// TODO WGJA WIP: 		 * secondary queue.  Send a throttle command or abort
// TODO WGJA WIP: 		 * if necessary.
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		c = LEFT(&tty->secondary);
// TODO WGJA WIP: 		if (tty->throttle && (c < SQ_THRESHOLD_LW)
// TODO WGJA WIP: 		    && !set_bit(TTY_SQ_THROTTLED, &tty->flags))
// TODO WGJA WIP: 			tty->throttle(tty, TTY_THROTTLE_SQ_FULL);
// TODO WGJA WIP: 		if (c == 0)
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		save_flags(flags); cli();
// TODO WGJA WIP: 		if (tty->read_q.tail != tty->read_q.head) {
// TODO WGJA WIP: 			c = 0xff & tty->read_q.buf[tty->read_q.tail];
// TODO WGJA WIP: 			special_flag = clear_bit(tty->read_q.tail,
// TODO WGJA WIP: 						  &tty->readq_flags);
// TODO WGJA WIP: 			tty->read_q.tail = (tty->read_q.tail + 1) &
// TODO WGJA WIP: 				(TTY_BUF_SIZE-1);
// TODO WGJA WIP: 			restore_flags(flags);
// TODO WGJA WIP: 		} else {
// TODO WGJA WIP: 			restore_flags(flags);
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (special_flag) {
// TODO WGJA WIP: 			tty->char_error = c & 3;
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (tty->char_error) {
// TODO WGJA WIP: 			if (tty->char_error == TTY_BREAK) {
// TODO WGJA WIP: 				tty->char_error = 0;
// TODO WGJA WIP: 				if (I_IGNBRK(tty))
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				if (I_PARMRK(tty)) {
// TODO WGJA WIP: 					put_tty_queue('\377', &tty->secondary);
// TODO WGJA WIP: 					put_tty_queue('\0', &tty->secondary);
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				put_tty_queue('\0', &tty->secondary);
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			/* If not a break, then a parity or frame error */
// TODO WGJA WIP: 			tty->char_error = 0;
// TODO WGJA WIP: 			if (I_IGNPAR(tty)) {
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			if (I_PARMRK(tty)) {
// TODO WGJA WIP: 				put_tty_queue('\377', &tty->secondary);
// TODO WGJA WIP: 				put_tty_queue('\0', &tty->secondary);
// TODO WGJA WIP: 				put_tty_queue(c, &tty->secondary);
// TODO WGJA WIP: 			} else
// TODO WGJA WIP: 				put_tty_queue('\0', &tty->secondary);
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (I_STRP(tty))
// TODO WGJA WIP: 			c &= 0x7f;
// TODO WGJA WIP: 		else if (I_PARMRK(tty) && (c == '\377'))
// TODO WGJA WIP: 			put_tty_queue('\377', &tty->secondary);
// TODO WGJA WIP: 		if (c==13) {
// TODO WGJA WIP: 			if (I_CRNL(tty))
// TODO WGJA WIP: 				c=10;
// TODO WGJA WIP: 			else if (I_NOCR(tty))
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 		} else if (c==10 && I_NLCR(tty))
// TODO WGJA WIP: 			c=13;
// TODO WGJA WIP: 		if (I_UCLC(tty))
// TODO WGJA WIP: 			c=tolower(c);
// TODO WGJA WIP: 		if (c == __DISABLED_CHAR)
// TODO WGJA WIP: 			tty->lnext = 1;
// TODO WGJA WIP: 		if (L_CANON(tty) && !tty->lnext) {
// TODO WGJA WIP: 			if (c == KILL_CHAR(tty) || c == WERASE_CHAR(tty)) {
// TODO WGJA WIP: 				int seen_alnums =
// TODO WGJA WIP: 				  (c == WERASE_CHAR(tty)) ? 0 : -1;
// TODO WGJA WIP: 
// TODO WGJA WIP: 				/* deal with killing the input line */
// TODO WGJA WIP: 				while(!(EMPTY(&tty->secondary) ||
// TODO WGJA WIP: 					(c=LAST(&tty->secondary))==10 ||
// TODO WGJA WIP: 					((EOF_CHAR(tty) != __DISABLED_CHAR) &&
// TODO WGJA WIP: 					 (c==EOF_CHAR(tty))))) {
// TODO WGJA WIP: 					/* if killing just a word, kill all
// TODO WGJA WIP: 					   non-alnum chars, then all alnum
// TODO WGJA WIP: 					   chars.  */
// TODO WGJA WIP: 					if (seen_alnums >= 0) {
// TODO WGJA WIP: 						if (isalnum(c))
// TODO WGJA WIP: 							seen_alnums++;
// TODO WGJA WIP: 						else if (seen_alnums)
// TODO WGJA WIP: 							break;
// TODO WGJA WIP: 					}
// TODO WGJA WIP: 					if (L_ECHO(tty)) {
// TODO WGJA WIP: 						if (c<32) {
// TODO WGJA WIP: 							put_tty_queue('\b', &tty->write_q);
// TODO WGJA WIP: 							put_tty_queue(' ', &tty->write_q);
// TODO WGJA WIP: 							put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 						}
// TODO WGJA WIP: 						put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 						put_tty_queue(' ',&tty->write_q);
// TODO WGJA WIP: 						put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 					}
// TODO WGJA WIP: 					DEC(tty->secondary.head);
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			if (c == ERASE_CHAR(tty)) {
// TODO WGJA WIP: 				if (EMPTY(&tty->secondary) ||
// TODO WGJA WIP: 				   (c=LAST(&tty->secondary))==10 ||
// TODO WGJA WIP: 				   ((EOF_CHAR(tty) != __DISABLED_CHAR) &&
// TODO WGJA WIP: 				    (c==EOF_CHAR(tty))))
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				if (L_ECHO(tty)) {
// TODO WGJA WIP: 					if (c<32) {
// TODO WGJA WIP: 						put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 						put_tty_queue(' ',&tty->write_q);
// TODO WGJA WIP: 						put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 					}
// TODO WGJA WIP: 					put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 					put_tty_queue(' ',&tty->write_q);
// TODO WGJA WIP: 					put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				DEC(tty->secondary.head);
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			if (c == LNEXT_CHAR(tty)) {
// TODO WGJA WIP: 				tty->lnext = 1;
// TODO WGJA WIP: 				if (L_ECHO(tty)) {
// TODO WGJA WIP: 					put_tty_queue('^',&tty->write_q);
// TODO WGJA WIP: 					put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (I_IXON(tty) && !tty->lnext) {
// TODO WGJA WIP: 			if (c == STOP_CHAR(tty)) {
// TODO WGJA WIP: 			        tty->status_changed = 1;
// TODO WGJA WIP: 				tty->ctrl_status |= TIOCPKT_STOP;
// TODO WGJA WIP: 				tty->stopped=1;
// TODO WGJA WIP: 				if (tty->stop)
// TODO WGJA WIP: 					(tty->stop)(tty);
// TODO WGJA WIP: 				if (IS_A_CONSOLE(tty->line)) {
// TODO WGJA WIP: 					set_vc_kbd_flag(kbd_table + fg_console, VC_SCROLLOCK);
// TODO WGJA WIP: 					set_leds();
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			if (((I_IXANY(tty)) && tty->stopped) ||
// TODO WGJA WIP: 			    (c == START_CHAR(tty))) {
// TODO WGJA WIP: 			        tty->status_changed = 1;
// TODO WGJA WIP: 				tty->ctrl_status |= TIOCPKT_START;
// TODO WGJA WIP: 				tty->stopped=0;
// TODO WGJA WIP: 				if (tty->start)
// TODO WGJA WIP: 					(tty->start)(tty);
// TODO WGJA WIP: 				if (IS_A_CONSOLE(tty->line)) {
// TODO WGJA WIP: 					clr_vc_kbd_flag(kbd_table + fg_console, VC_SCROLLOCK);
// TODO WGJA WIP: 					set_leds();
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (L_ISIG(tty) && !tty->lnext) {
// TODO WGJA WIP: 			if (c == INTR_CHAR(tty)) {
// TODO WGJA WIP: 				kill_pg(tty->pgrp, SIGINT, 1);
// TODO WGJA WIP: 				flush_input(tty);
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			if (c == QUIT_CHAR(tty)) {
// TODO WGJA WIP: 				kill_pg(tty->pgrp, SIGQUIT, 1);
// TODO WGJA WIP: 				flush_input(tty);
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			if (c == SUSPEND_CHAR(tty)) {
// TODO WGJA WIP: 				if (!is_orphaned_pgrp(tty->pgrp)) {
// TODO WGJA WIP: 					kill_pg(tty->pgrp, SIGTSTP, 1);
// TODO WGJA WIP: 					flush_input(tty);
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (c==10 || (EOF_CHAR(tty) != __DISABLED_CHAR &&
// TODO WGJA WIP: 		    c==EOF_CHAR(tty)))
// TODO WGJA WIP: 			tty->secondary.data++;
// TODO WGJA WIP: 		if ((c==10) && (L_ECHO(tty) || (L_CANON(tty) && L_ECHONL(tty)))) {
// TODO WGJA WIP: 			put_tty_queue('\n',&tty->write_q);
// TODO WGJA WIP: 			put_tty_queue('\r',&tty->write_q);
// TODO WGJA WIP: 		} else if (L_ECHO(tty)) {
// TODO WGJA WIP: 			if (c<32 && L_ECHOCTL(tty)) {
// TODO WGJA WIP: 				put_tty_queue('^',&tty->write_q);
// TODO WGJA WIP: 				put_tty_queue(c+'A'-1, &tty->write_q);
// TODO WGJA WIP: 				if (EOF_CHAR(tty) != __DISABLED_CHAR &&
// TODO WGJA WIP: 				    c==EOF_CHAR(tty) && !tty->lnext) {
// TODO WGJA WIP: 					put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 					put_tty_queue('\b',&tty->write_q);
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 			} else
// TODO WGJA WIP: 				put_tty_queue(c, &tty->write_q);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		tty->lnext = 0;
// TODO WGJA WIP: 		put_tty_queue(c, &tty->secondary);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	TTY_WRITE_FLUSH(tty);
// TODO WGJA WIP: 	if (!EMPTY(&tty->secondary))
// TODO WGJA WIP: 		wake_up_interruptible(&tty->secondary.proc_list);
// TODO WGJA WIP: 	if (tty->write_q.proc_list && LEFT(&tty->write_q) > TTY_BUF_SIZE/2)
// TODO WGJA WIP: 		wake_up_interruptible(&tty->write_q.proc_list);
// TODO WGJA WIP: 	if (tty->throttle && (LEFT(&tty->read_q) >= RQ_THRESHOLD_HW)
// TODO WGJA WIP: 	    && clear_bit(TTY_RQ_THROTTLED, &tty->flags))
// TODO WGJA WIP: 		tty->throttle(tty, TTY_THROTTLE_RQ_AVAIL);
// TODO WGJA WIP: 	if (tty->throttle && (LEFT(&tty->secondary) >= SQ_THRESHOLD_HW)
// TODO WGJA WIP: 	    && clear_bit(TTY_SQ_THROTTLED, &tty->flags))
// TODO WGJA WIP: 		tty->throttle(tty, TTY_THROTTLE_SQ_AVAIL);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int is_ignored(int sig)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return ((current->blocked & (1<<(sig-1))) ||
// TODO WGJA WIP: 	        (current->sigaction[sig-1].sa_handler == SIG_IGN));
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int available_canon_input(struct tty_struct *);
// TODO WGJA WIP: static void __wait_for_canon_input(struct file * file, struct tty_struct *);
// TODO WGJA WIP: 
// TODO WGJA WIP: static void wait_for_canon_input(struct file * file, struct tty_struct * tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (!available_canon_input(tty)) {
// TODO WGJA WIP: 		if (current->signal & ~current->blocked)
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		__wait_for_canon_input(file, tty);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int read_chan(struct tty_struct * tty, struct file * file, char * buf, int nr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct wait_queue wait = { current, NULL };
// TODO WGJA WIP: 	int c;
// TODO WGJA WIP: 	char * b=buf;
// TODO WGJA WIP: 	int minimum,time;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (L_CANON(tty))
// TODO WGJA WIP: 		minimum = time = current->timeout = 0;
// TODO WGJA WIP: 	else {
// TODO WGJA WIP: 		time = 10L*tty->termios->c_cc[VTIME];
// TODO WGJA WIP: 		minimum = tty->termios->c_cc[VMIN];
// TODO WGJA WIP: 		if (minimum)
// TODO WGJA WIP: 			current->timeout = 0xffffffff;
// TODO WGJA WIP: 		else {
// TODO WGJA WIP: 			if (time)
// TODO WGJA WIP: 				current->timeout = time + jiffies;
// TODO WGJA WIP: 			else
// TODO WGJA WIP: 				current->timeout = 0;
// TODO WGJA WIP: 			time = 0;
// TODO WGJA WIP: 			minimum = 1;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (file->f_flags & O_NONBLOCK) {
// TODO WGJA WIP: 		time = current->timeout = 0;
// TODO WGJA WIP: 		if (L_CANON(tty) && !available_canon_input(tty))
// TODO WGJA WIP: 				return -EAGAIN;
// TODO WGJA WIP: 	} else if (L_CANON(tty)) {
// TODO WGJA WIP: 		wait_for_canon_input(file, tty);
// TODO WGJA WIP: 		if (current->signal & ~current->blocked)
// TODO WGJA WIP: 			return -ERESTARTSYS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (minimum>nr)
// TODO WGJA WIP: 		minimum = nr;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* deal with packet mode:  First test for status change */
// TODO WGJA WIP: 	if (tty->packet && tty->link && tty->link->status_changed) {
// TODO WGJA WIP: 		put_fs_byte (tty->link->ctrl_status, b);
// TODO WGJA WIP: 		tty->link->status_changed = 0;
// TODO WGJA WIP: 		return 1;
// TODO WGJA WIP: 	}
	  // TODO WGJA WIP: 
// TODO WGJA WIP: 	/* now bump the buffer up one. */
// TODO WGJA WIP: 	if (tty->packet) {
// TODO WGJA WIP: 		put_fs_byte (0,b++);
// TODO WGJA WIP: 		nr--;
// TODO WGJA WIP: 		/* this really shouldn't happen, but we need to 
// TODO WGJA WIP: 		put it here. */
// TODO WGJA WIP: 		if (nr == 0)
// TODO WGJA WIP: 			return 1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	add_wait_queue(&tty->secondary.proc_list, &wait);
// TODO WGJA WIP: 	while (nr>0) {
// TODO WGJA WIP: 		if (tty_hung_up_p(file)) {
// TODO WGJA WIP: 			file->f_flags &= ~O_NONBLOCK;
// TODO WGJA WIP: 			break;  /* force read() to return 0 */
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		TTY_READ_FLUSH(tty);
// TODO WGJA WIP: 		if (tty->link)
// TODO WGJA WIP: 			TTY_WRITE_FLUSH(tty->link);
// TODO WGJA WIP: 		while (nr > 0 && ((c = get_tty_queue(&tty->secondary)) >= 0)) {
// TODO WGJA WIP: 			if ((EOF_CHAR(tty) != __DISABLED_CHAR &&
// TODO WGJA WIP: 			     c==EOF_CHAR(tty)) || c==10)
// TODO WGJA WIP: 				tty->secondary.data--;
// TODO WGJA WIP: 			if ((EOF_CHAR(tty) != __DISABLED_CHAR &&
// TODO WGJA WIP: 			     c==EOF_CHAR(tty)) && L_CANON(tty))
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 			put_fs_byte(c,b++);
// TODO WGJA WIP: 			nr--;
// TODO WGJA WIP: 			if (time)
// TODO WGJA WIP: 				current->timeout = time+jiffies;
// TODO WGJA WIP: 			if (c==10 && L_CANON(tty))
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 		};
// TODO WGJA WIP: 		wake_up_interruptible(&tty->read_q.proc_list);
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * If there is enough space in the secondary queue
// TODO WGJA WIP: 		 * now, let the low-level driver know.
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		if (tty->throttle && (LEFT(&tty->secondary) >= SQ_THRESHOLD_HW)
// TODO WGJA WIP: 		    && clear_bit(TTY_SQ_THROTTLED, &tty->flags))
// TODO WGJA WIP: 			tty->throttle(tty, TTY_THROTTLE_SQ_AVAIL);
// TODO WGJA WIP: 		if (tty->link) {
// TODO WGJA WIP: 			if (IS_A_PTY_MASTER(tty->line)) {
// TODO WGJA WIP: 				if ((tty->flags & (1 << TTY_SLAVE_OPENED))
// TODO WGJA WIP: 				    && tty->link->count <= 1) {
// TODO WGJA WIP: 					file->f_flags &= ~O_NONBLOCK;
// TODO WGJA WIP: 					break;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 			} else if (!tty->link->count) {
// TODO WGJA WIP: 				file->f_flags &= ~O_NONBLOCK;
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (b-buf >= minimum || !current->timeout)
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		if (current->signal & ~current->blocked) 
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		TTY_READ_FLUSH(tty);
// TODO WGJA WIP: 		if (tty->link)
// TODO WGJA WIP: 			TTY_WRITE_FLUSH(tty->link);
// TODO WGJA WIP: 		if (!EMPTY(&tty->secondary))
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		current->state = TASK_INTERRUPTIBLE;
// TODO WGJA WIP: 		if (EMPTY(&tty->secondary))
// TODO WGJA WIP: 			schedule();
// TODO WGJA WIP: 		current->state = TASK_RUNNING;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	remove_wait_queue(&tty->secondary.proc_list, &wait);
// TODO WGJA WIP: 	TTY_READ_FLUSH(tty);
// TODO WGJA WIP: 	if (tty->link && tty->link->write)
// TODO WGJA WIP: 		TTY_WRITE_FLUSH(tty->link);
// TODO WGJA WIP: 	current->timeout = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* packet mode sticks in an extra 0.  If that's all we've got,
// TODO WGJA WIP: 	   we should count it a zero bytes. */
// TODO WGJA WIP: 	if (tty->packet) {
// TODO WGJA WIP: 		if ((b-buf) > 1)
// TODO WGJA WIP: 			return b-buf;
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		if (b-buf)
// TODO WGJA WIP: 			return b-buf;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (current->signal & ~current->blocked)
// TODO WGJA WIP: 		return -ERESTARTSYS;
// TODO WGJA WIP: 	if (file->f_flags & O_NONBLOCK)
// TODO WGJA WIP: 		return -EAGAIN;
// TODO WGJA WIP: 	if (IS_A_PTY_MASTER(tty->line))
// TODO WGJA WIP: 		return -EIO;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static void __wait_for_canon_input(struct file * file, struct tty_struct * tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct wait_queue wait = { current, NULL };
// TODO WGJA WIP: 
// TODO WGJA WIP: 	add_wait_queue(&tty->secondary.proc_list, &wait);
// TODO WGJA WIP: 	while (1) {
// TODO WGJA WIP: 		current->state = TASK_INTERRUPTIBLE;
// TODO WGJA WIP: 		if (available_canon_input(tty))
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		if (current->signal & ~current->blocked)
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		if (tty_hung_up_p(file))
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		schedule();
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	current->state = TASK_RUNNING;
// TODO WGJA WIP: 	remove_wait_queue(&tty->secondary.proc_list, &wait);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int available_canon_input(struct tty_struct * tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	TTY_READ_FLUSH(tty);
// TODO WGJA WIP: 	if (tty->link)
// TODO WGJA WIP: 		if (tty->link->count)
// TODO WGJA WIP: 			TTY_WRITE_FLUSH(tty->link);
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			return 1;
// TODO WGJA WIP: 	if (FULL(&tty->read_q))
// TODO WGJA WIP: 		return 1;
// TODO WGJA WIP: 	if (tty->secondary.data)
// TODO WGJA WIP: 		return 1;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int write_chan(struct tty_struct * tty, struct file * file, char * buf, int nr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct wait_queue wait = { current, NULL };
// TODO WGJA WIP: 	char c, *b=buf;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (nr < 0)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	if (!nr)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	add_wait_queue(&tty->write_q.proc_list, &wait);
// TODO WGJA WIP: 	while (nr>0) {
// TODO WGJA WIP: 		if (current->signal & ~current->blocked)
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		if (tty_hung_up_p(file))
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		if (tty->link && !tty->link->count) {
// TODO WGJA WIP: 			send_sig(SIGPIPE,current,0);
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		current->state = TASK_INTERRUPTIBLE;
// TODO WGJA WIP: 		if (FULL(&tty->write_q)) {
// TODO WGJA WIP: 			TTY_WRITE_FLUSH(tty);
// TODO WGJA WIP: 			if (FULL(&tty->write_q))
// TODO WGJA WIP: 				schedule();
// TODO WGJA WIP: 			current->state = TASK_RUNNING;
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		current->state = TASK_RUNNING;
// TODO WGJA WIP: 		while (nr>0 && !FULL(&tty->write_q)) {
// TODO WGJA WIP: 			c=get_fs_byte(b);
// TODO WGJA WIP: 			if (O_POST(tty)) {
// TODO WGJA WIP: 				switch (c) {
// TODO WGJA WIP: 					case '\n':
// TODO WGJA WIP: 						if (O_NLRET(tty)) {
// TODO WGJA WIP: 							tty->column = 0;
// TODO WGJA WIP: 						}
// TODO WGJA WIP: 						if (O_NLCR(tty)) {
// TODO WGJA WIP: 							if (!set_bit(TTY_CR_PENDING,&tty->flags)) {
// TODO WGJA WIP: 								c = '\r';
// TODO WGJA WIP: 								tty->column = 0;
// TODO WGJA WIP: 								b--; nr++;
// TODO WGJA WIP: 							} else {
// TODO WGJA WIP: 								clear_bit(TTY_CR_PENDING,&tty->flags);
// TODO WGJA WIP: 							}
// TODO WGJA WIP: 						}
// TODO WGJA WIP: 						break;
// TODO WGJA WIP: 					case '\r':
// TODO WGJA WIP: 						if (O_NOCR(tty) && tty->column == 0) {
// TODO WGJA WIP: 							b++; nr--;
// TODO WGJA WIP: 							continue;
// TODO WGJA WIP: 						}
// TODO WGJA WIP: 						if (O_CRNL(tty)) {
// TODO WGJA WIP: 							c = '\n';
// TODO WGJA WIP: 							if (O_NLRET(tty))
// TODO WGJA WIP: 								tty->column = 0;
// TODO WGJA WIP: 							break;
// TODO WGJA WIP: 						}
// TODO WGJA WIP: 						tty->column = 0;
// TODO WGJA WIP: 						break;
// TODO WGJA WIP: 					case '\t':
// TODO WGJA WIP: 						if (O_TABDLY(tty) == XTABS) {
// TODO WGJA WIP: 							c = ' ';
// TODO WGJA WIP: 							tty->column++;
// TODO WGJA WIP: 							if (tty->column % 8 != 0) {
// TODO WGJA WIP: 								b--; nr++;
// TODO WGJA WIP: 							}
// TODO WGJA WIP: 						}
// TODO WGJA WIP: 						break;
// TODO WGJA WIP: 					case '\b':
// TODO WGJA WIP: 						tty->column--;
// TODO WGJA WIP: 						break;
// TODO WGJA WIP: 					default:
// TODO WGJA WIP: 						if (O_LCUC(tty))
// TODO WGJA WIP: 							c = toupper(c);
// TODO WGJA WIP: 						tty->column++;
// TODO WGJA WIP: 						break;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			b++; nr--;
// TODO WGJA WIP: 			put_tty_queue(c,&tty->write_q);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (need_resched)
// TODO WGJA WIP: 			schedule();
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	remove_wait_queue(&tty->write_q.proc_list, &wait);
// TODO WGJA WIP: 	TTY_WRITE_FLUSH(tty);
// TODO WGJA WIP: 	if (b-buf)
// TODO WGJA WIP: 		return b-buf;
// TODO WGJA WIP: 	if (tty->link && !tty->link->count)
// TODO WGJA WIP: 		return -EPIPE;
// TODO WGJA WIP: 	if (current->signal & ~current->blocked)
// TODO WGJA WIP: 		return -ERESTARTSYS;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int tty_read(struct inode * inode, struct file * file, char * buf, int count)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i, dev;
// TODO WGJA WIP: 	struct tty_struct * tty;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	dev = file->f_rdev;
// TODO WGJA WIP: 	if (MAJOR(dev) != 4) {
// TODO WGJA WIP: 		printk("tty_read: bad pseudo-major nr #%d\n", MAJOR(dev));
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	dev = MINOR(dev);
// TODO WGJA WIP: 	tty = TTY_TABLE(dev);
// TODO WGJA WIP: 	if (!tty || (tty->flags & (1 << TTY_IO_ERROR)))
// TODO WGJA WIP: 		return -EIO;
// TODO WGJA WIP: 	if ((inode->i_rdev != 0x0400) && /* don't stop on /dev/console */
// TODO WGJA WIP: 	    (tty->pgrp > 0) &&
// TODO WGJA WIP: 	    (current->tty == dev) &&
// TODO WGJA WIP: 	    (tty->pgrp != current->pgrp))
// TODO WGJA WIP: 		if (is_ignored(SIGTTIN) || is_orphaned_pgrp(current->pgrp))
// TODO WGJA WIP: 			return -EIO;
// TODO WGJA WIP: 		else {
// TODO WGJA WIP: 			(void) kill_pg(current->pgrp, SIGTTIN, 1);
// TODO WGJA WIP: 			return -ERESTARTSYS;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	if (ldiscs[tty->disc].read)
// TODO WGJA WIP: 		i = (ldiscs[tty->disc].read)(tty,file,buf,count);
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		i = -EIO;
// TODO WGJA WIP: 	if (i > 0)
// TODO WGJA WIP: 		inode->i_atime = CURRENT_TIME;
// TODO WGJA WIP: 	return i;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int tty_write(struct inode * inode, struct file * file, char * buf, int count)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int dev, i, is_console;
// TODO WGJA WIP: 	struct tty_struct * tty;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	dev = file->f_rdev;
// TODO WGJA WIP: 	is_console = (inode->i_rdev == 0x0400);
// TODO WGJA WIP: 	if (MAJOR(dev) != 4) {
// TODO WGJA WIP: 		printk("tty_write: pseudo-major != 4\n");
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	dev = MINOR(dev);
// TODO WGJA WIP: 	if (is_console && redirect)
// TODO WGJA WIP: 		tty = redirect;
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		tty = TTY_TABLE(dev);
// TODO WGJA WIP: 	if (!tty || !tty->write || (tty->flags & (1 << TTY_IO_ERROR)))
// TODO WGJA WIP: 		return -EIO;
// TODO WGJA WIP: 	if (!is_console && L_TOSTOP(tty) && (tty->pgrp > 0) &&
// TODO WGJA WIP: 	    (current->tty == dev) && (tty->pgrp != current->pgrp)) {
// TODO WGJA WIP: 		if (is_orphaned_pgrp(current->pgrp))
// TODO WGJA WIP: 			return -EIO;
// TODO WGJA WIP: 		if (!is_ignored(SIGTTOU)) {
// TODO WGJA WIP: 			(void) kill_pg(current->pgrp, SIGTTOU, 1);
// TODO WGJA WIP: 			return -ERESTARTSYS;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (ldiscs[tty->disc].write)
// TODO WGJA WIP: 		i = (ldiscs[tty->disc].write)(tty,file,buf,count);
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		i = -EIO;
// TODO WGJA WIP: 	if (i > 0)
// TODO WGJA WIP: 		inode->i_mtime = CURRENT_TIME;
// TODO WGJA WIP: 	return i;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This is so ripe with races that you should *really* not touch this
// TODO WGJA WIP:  * unless you know exactly what you are doing. All the changes have to be
// TODO WGJA WIP:  * made atomically, or there may be incorrect pointers all over the place.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static int init_dev(int dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct tty_struct *tty, *o_tty;
// TODO WGJA WIP: 	struct termios *tp, *o_tp, *ltp, *o_ltp;
// TODO WGJA WIP: 	int retval;
// TODO WGJA WIP: 	int o_dev;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	o_dev = PTY_OTHER(dev);
// TODO WGJA WIP: 	tty = o_tty = NULL;
// TODO WGJA WIP: 	tp = o_tp = NULL;
// TODO WGJA WIP: 	ltp = o_ltp = NULL;
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 	retval = -EAGAIN;
// TODO WGJA WIP: 	if (IS_A_PTY_MASTER(dev) && tty_table[dev] && tty_table[dev]->count)
// TODO WGJA WIP: 		goto end_init;
// TODO WGJA WIP: 	retval = -ENOMEM;
// TODO WGJA WIP: 	if (!tty_table[dev] && !tty) {
// TODO WGJA WIP: 		if (!(tty = (struct tty_struct*) get_free_page(GFP_KERNEL)))
// TODO WGJA WIP: 			goto end_init;
// TODO WGJA WIP: 		initialize_tty_struct(dev, tty);
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!tty_termios[dev] && !tp) {
// TODO WGJA WIP: 		tp = (struct termios *) kmalloc(sizeof(struct termios),
// TODO WGJA WIP: 						GFP_KERNEL);
// TODO WGJA WIP: 		if (!tp)
// TODO WGJA WIP: 			goto end_init;
// TODO WGJA WIP: 		initialize_termios(dev, tp);
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!termios_locked[dev] && !ltp) {
// TODO WGJA WIP: 		ltp = (struct termios *) kmalloc(sizeof(struct termios),
// TODO WGJA WIP: 						 GFP_KERNEL);
// TODO WGJA WIP: 		if (!ltp)
// TODO WGJA WIP: 			goto end_init;
// TODO WGJA WIP: 		memset(ltp, 0, sizeof(struct termios));
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_A_PTY(dev)) {
// TODO WGJA WIP: 		if (!tty_table[o_dev] && !o_tty) {
// TODO WGJA WIP: 			o_tty = (struct tty_struct *)
// TODO WGJA WIP: 				get_free_page(GFP_KERNEL);
// TODO WGJA WIP: 			if (!o_tty)
// TODO WGJA WIP: 				goto end_init;
// TODO WGJA WIP: 			initialize_tty_struct(o_dev, o_tty);
// TODO WGJA WIP: 			goto repeat;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!tty_termios[o_dev] && !o_tp) {
// TODO WGJA WIP: 			o_tp = (struct termios *)
// TODO WGJA WIP: 				kmalloc(sizeof(struct termios), GFP_KERNEL);
// TODO WGJA WIP: 			if (!o_tp)
// TODO WGJA WIP: 				goto end_init;
// TODO WGJA WIP: 			initialize_termios(o_dev, o_tp);
// TODO WGJA WIP: 			goto repeat;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!termios_locked[o_dev] && !o_ltp) {
// TODO WGJA WIP: 			o_ltp = (struct termios *)
// TODO WGJA WIP: 				kmalloc(sizeof(struct termios), GFP_KERNEL);
// TODO WGJA WIP: 			if (!o_ltp)
// TODO WGJA WIP: 				goto end_init;
// TODO WGJA WIP: 			memset(o_ltp, 0, sizeof(struct termios));
// TODO WGJA WIP: 			goto repeat;
// TODO WGJA WIP: 		}
		// TODO WGJA WIP: 
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	/* Now we have allocated all the structures: update all the pointers.. */
// TODO WGJA WIP: 	if (!tty_termios[dev]) {
// TODO WGJA WIP: 		tty_termios[dev] = tp;
// TODO WGJA WIP: 		tp = NULL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!tty_table[dev]) {
// TODO WGJA WIP: 		tty->termios = tty_termios[dev];
// TODO WGJA WIP: 		tty_table[dev] = tty;
// TODO WGJA WIP: 		tty = NULL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!termios_locked[dev]) {
// TODO WGJA WIP: 		termios_locked[dev] = ltp;
// TODO WGJA WIP: 		ltp = NULL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_A_PTY(dev)) {
// TODO WGJA WIP: 		if (!tty_termios[o_dev]) {
// TODO WGJA WIP: 			tty_termios[o_dev] = o_tp;
// TODO WGJA WIP: 			o_tp = NULL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!termios_locked[o_dev]) {
// TODO WGJA WIP: 			termios_locked[o_dev] = o_ltp;
// TODO WGJA WIP: 			o_ltp = NULL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!tty_table[o_dev]) {
// TODO WGJA WIP: 			o_tty->termios = tty_termios[o_dev];
// TODO WGJA WIP: 			tty_table[o_dev] = o_tty;
// TODO WGJA WIP: 			o_tty = NULL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		tty_table[dev]->link = tty_table[o_dev];
// TODO WGJA WIP: 		tty_table[o_dev]->link = tty_table[dev];
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	tty_table[dev]->count++;
// TODO WGJA WIP: 	if (IS_A_PTY_MASTER(dev))
// TODO WGJA WIP: 		tty_table[o_dev]->count++;
// TODO WGJA WIP: 	retval = 0;
// TODO WGJA WIP: end_init:
// TODO WGJA WIP: 	if (tty)
// TODO WGJA WIP: 		free_page((unsigned long) tty);
// TODO WGJA WIP: 	if (o_tty)
// TODO WGJA WIP: 		free_page((unsigned long) o_tty);
// TODO WGJA WIP: 	if (tp)
// TODO WGJA WIP: 		kfree_s(tp, sizeof(struct termios));
// TODO WGJA WIP: 	if (o_tp)
// TODO WGJA WIP: 		kfree_s(o_tp, sizeof(struct termios));
// TODO WGJA WIP: 	if (ltp)
// TODO WGJA WIP: 		kfree_s(ltp, sizeof(struct termios));
// TODO WGJA WIP: 	if (o_ltp)
// TODO WGJA WIP: 		kfree_s(o_ltp, sizeof(struct termios));
// TODO WGJA WIP: 	return retval;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Even releasing the tty structures is a tricky business.. We have
// TODO WGJA WIP:  * to be very careful that the structures are all released at the
// TODO WGJA WIP:  * same time, as interrupts might otherwise get the wrong pointers.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static void release_dev(int dev, struct file * filp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct tty_struct *tty, *o_tty;
// TODO WGJA WIP: 	struct termios *tp, *o_tp;
// TODO WGJA WIP: 	struct task_struct **p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	tty = tty_table[dev];
// TODO WGJA WIP: 	tp = tty_termios[dev];
// TODO WGJA WIP: 	o_tty = NULL;
// TODO WGJA WIP: 	o_tp = NULL;
// TODO WGJA WIP: 	if (!tty) {
// TODO WGJA WIP: 		printk("release_dev: tty_table[%d] was NULL\n", dev);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!tp) {
// TODO WGJA WIP: 		printk("release_dev: tty_termios[%d] was NULL\n", dev);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_A_PTY(dev)) {
// TODO WGJA WIP: 		o_tty = tty_table[PTY_OTHER(dev)];
// TODO WGJA WIP: 		o_tp = tty_termios[PTY_OTHER(dev)];
// TODO WGJA WIP: 		if (!o_tty) {
// TODO WGJA WIP: 			printk("release_dev: pty pair(%d) was NULL\n", dev);
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!o_tp) {
// TODO WGJA WIP: 			printk("release_dev: pty pair(%d) termios was NULL\n", dev);
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (tty->link != o_tty || o_tty->link != tty) {
// TODO WGJA WIP: 			printk("release_dev: bad pty pointers\n");
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	tty->write_data_cnt = 0; /* Clear out pending trash */
// TODO WGJA WIP: 	if (tty->close)
// TODO WGJA WIP: 		tty->close(tty, filp);
// TODO WGJA WIP: 	if (IS_A_PTY_MASTER(dev)) {
// TODO WGJA WIP: 		if (--tty->link->count < 0) {
// TODO WGJA WIP: 			printk("release_dev: bad tty slave count (dev = %d): %d\n",
// TODO WGJA WIP: 			       dev, tty->count);
// TODO WGJA WIP: 			tty->link->count = 0;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (--tty->count < 0) {
// TODO WGJA WIP: 		printk("release_dev: bad tty_table[%d]->count: %d\n",
// TODO WGJA WIP: 		       dev, tty->count);
// TODO WGJA WIP: 		tty->count = 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (tty->count)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Make sure there aren't any processes that still think this
// TODO WGJA WIP: 	 * tty is their controlling tty.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
// TODO WGJA WIP: 		if ((*p) && (*p)->tty == tty->line)
// TODO WGJA WIP: 		(*p)->tty = -1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (ldiscs[tty->disc].close != NULL)
// TODO WGJA WIP: 		ldiscs[tty->disc].close(tty);
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (o_tty) {
// TODO WGJA WIP: 		if (o_tty->count)
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		else {
// TODO WGJA WIP: 			tty_table[PTY_OTHER(dev)] = NULL;
// TODO WGJA WIP: 			tty_termios[PTY_OTHER(dev)] = NULL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	tty_table[dev] = NULL;
// TODO WGJA WIP: 	if (IS_A_PTY(dev)) {
// TODO WGJA WIP: 		tty_termios[dev] = NULL;
// TODO WGJA WIP: 		kfree_s(tp, sizeof(struct termios));
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (tty == redirect || o_tty == redirect)
// TODO WGJA WIP: 		redirect = NULL;
// TODO WGJA WIP: 	free_page((unsigned long) tty);
// TODO WGJA WIP: 	if (o_tty)
// TODO WGJA WIP: 		free_page((unsigned long) o_tty);
// TODO WGJA WIP: 	if (o_tp)
// TODO WGJA WIP: 		kfree_s(o_tp, sizeof(struct termios));
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * tty_open and tty_release keep up the tty count that contains the
// TODO WGJA WIP:  * number of opens done on a tty. We cannot use the inode-count, as
// TODO WGJA WIP:  * different inodes might point to the same tty.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Open-counting is needed for pty masters, as well as for keeping
// TODO WGJA WIP:  * track of serial lines: DTR is dropped when the last close happens.
// TODO WGJA WIP:  * (This is not done solely through tty->count, now.  - Ted 1/27/92)
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * The termios state of a pty is reset on first open so that
// TODO WGJA WIP:  * settings don't persist across reuse.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static int tty_open(struct inode * inode, struct file * filp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct tty_struct *tty;
// TODO WGJA WIP: 	int major, minor;
// TODO WGJA WIP: 	int noctty, retval;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	minor = MINOR(inode->i_rdev);
// TODO WGJA WIP: 	major = MAJOR(inode->i_rdev);
// TODO WGJA WIP: 	noctty = filp->f_flags & O_NOCTTY;
// TODO WGJA WIP: 	if (major == 5) {
// TODO WGJA WIP: 		if (!minor) {
// TODO WGJA WIP: 			major = 4;
// TODO WGJA WIP: 			minor = current->tty;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		noctty = 1;
// TODO WGJA WIP: 	} else if (major == 4) {
// TODO WGJA WIP: 		if (!minor) {
// TODO WGJA WIP: 			minor = fg_console + 1;
// TODO WGJA WIP: 			noctty = 1;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		printk("Bad major #%d in tty_open\n", MAJOR(inode->i_rdev));
// TODO WGJA WIP: 		return -ENODEV;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (minor <= 0)
// TODO WGJA WIP: 		return -ENXIO;
// TODO WGJA WIP: 	if (IS_A_PTY_MASTER(minor))
// TODO WGJA WIP: 		noctty = 1;
// TODO WGJA WIP: 	filp->f_rdev = (major << 8) | minor;
// TODO WGJA WIP: 	retval = init_dev(minor);
// TODO WGJA WIP: 	if (retval)
// TODO WGJA WIP: 		return retval;
// TODO WGJA WIP: 	tty = tty_table[minor];
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* clean up the packet stuff. */
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 *  Why is this not done in init_dev?  Right here, if another 
// TODO WGJA WIP: 	 * process opens up a tty in packet mode, all the packet 
// TODO WGJA WIP: 	 * variables get cleared.  Come to think of it, is anything 
// TODO WGJA WIP: 	 * using the packet mode at all???  - Ted, 1/27/93
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	tty->status_changed = 0;
// TODO WGJA WIP: 	tty->ctrl_status = 0;
// TODO WGJA WIP: 	tty->packet = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (tty->open) {
// TODO WGJA WIP: 		retval = tty->open(tty, filp);
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		retval = -ENODEV;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (retval) {
// TODO WGJA WIP: 		release_dev(minor, filp);
// TODO WGJA WIP: 		return retval;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!noctty &&
// TODO WGJA WIP: 	    current->leader &&
// TODO WGJA WIP: 	    current->tty<0 &&
// TODO WGJA WIP: 	    tty->session==0) {
// TODO WGJA WIP: 		current->tty = minor;
// TODO WGJA WIP: 		tty->session = current->session;
// TODO WGJA WIP: 		tty->pgrp = current->pgrp;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	filp->f_rdev = 0x0400 | minor; /* Set it to something normal */
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Note that releasing a pty master also releases the child, so
// TODO WGJA WIP:  * we have to make the redirection checks after that and on both
// TODO WGJA WIP:  * sides of a pty.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static void tty_release(struct inode * inode, struct file * filp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int dev;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	dev = filp->f_rdev;
// TODO WGJA WIP: 	if (MAJOR(dev) != 4) {
// TODO WGJA WIP: 		printk("tty_release: tty pseudo-major != 4\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	dev = MINOR(filp->f_rdev);
// TODO WGJA WIP: 	if (!dev) {
// TODO WGJA WIP: 		printk("tty_release: bad f_rdev\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	release_dev(dev, filp);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int tty_select(struct inode * inode, struct file * filp, int sel_type, select_table * wait)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int dev;
// TODO WGJA WIP: 	struct tty_struct * tty;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	dev = filp->f_rdev;
// TODO WGJA WIP: 	if (MAJOR(dev) != 4) {
// TODO WGJA WIP: 		printk("tty_select: tty pseudo-major != 4\n");
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	dev = MINOR(filp->f_rdev);
// TODO WGJA WIP: 	tty = TTY_TABLE(dev);
// TODO WGJA WIP: 	if (!tty) {
// TODO WGJA WIP: 		printk("tty_select: tty struct for dev %d was NULL\n", dev);
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	switch (sel_type) {
// TODO WGJA WIP: 		case SEL_IN:
// TODO WGJA WIP: 			if (L_CANON(tty)) {
// TODO WGJA WIP: 				if (available_canon_input(tty))
// TODO WGJA WIP: 					return 1;
// TODO WGJA WIP: 			} else if (!EMPTY(&tty->secondary))
// TODO WGJA WIP: 				return 1;
// TODO WGJA WIP: 			if (tty->link) {
// TODO WGJA WIP: 				if (IS_A_PTY_MASTER(tty->line)) {
// TODO WGJA WIP: 					if ((tty->flags & (1 << TTY_SLAVE_OPENED))
// TODO WGJA WIP: 					    && tty->link->count <= 1)
// TODO WGJA WIP: 						return 1;
// TODO WGJA WIP: 				} else {
// TODO WGJA WIP: 					if (!tty->link->count)
// TODO WGJA WIP: 						return 1;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 
// TODO WGJA WIP: 			/* see if the status byte can be read. */
// TODO WGJA WIP: 			if (tty->packet && tty->link &&
// TODO WGJA WIP: 			    tty->link->status_changed)
// TODO WGJA WIP: 				return 1;
// TODO WGJA WIP: 
// TODO WGJA WIP: 			select_wait(&tty->secondary.proc_list, wait);
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		case SEL_OUT:
// TODO WGJA WIP: 			if (!FULL(&tty->write_q))
// TODO WGJA WIP: 				return 1;
// TODO WGJA WIP: 			select_wait(&tty->write_q.proc_list, wait);
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		case SEL_EX:
// TODO WGJA WIP: 			if (tty->link) {
// TODO WGJA WIP: 				if (IS_A_PTY_MASTER(tty->line)) {
// TODO WGJA WIP: 					if ((tty->flags & (1 << TTY_SLAVE_OPENED))
// TODO WGJA WIP: 					    && tty->link->count <= 1)
// TODO WGJA WIP: 						return 1;
// TODO WGJA WIP: 				} else {
// TODO WGJA WIP: 					if (!tty->link->count)
// TODO WGJA WIP: 						return 1;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This implements the "Secure Attention Key" ---  the idea is to
// TODO WGJA WIP:  * prevent trojan horses by killing all processes associated with this
// TODO WGJA WIP:  * tty when the user hits the "Secure Attention Key".  Required for
// TODO WGJA WIP:  * super-paranoid applications --- see the Orange Book for more details.
// TODO WGJA WIP:  * 
// TODO WGJA WIP:  * This code could be nicer; ideally it should send a HUP, wait a few
// TODO WGJA WIP:  * seconds, then send a INT, and then a KILL signal.  But you then
// TODO WGJA WIP:  * have to coordinate with the init process, since all processes associated
// TODO WGJA WIP:  * with the current tty must be dead before the new getty is allowed
// TODO WGJA WIP:  * to spawn.
// TODO WGJA WIP:  */
// TODO WGJA WIP: void do_SAK( struct tty_struct *tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct task_struct **p;
// TODO WGJA WIP: 	int line = tty->line;
// TODO WGJA WIP: 	int session = tty->session;
// TODO WGJA WIP: 	int		i;
// TODO WGJA WIP: 	struct file	*filp;
	// TODO WGJA WIP: 
// TODO WGJA WIP: 	flush_input(tty);
// TODO WGJA WIP: 	flush_output(tty);
// TODO WGJA WIP:  	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
// TODO WGJA WIP: 		if (!(*p))
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (((*p)->tty == line) ||
// TODO WGJA WIP: 		    ((session > 0) && ((*p)->session == session)))
// TODO WGJA WIP: 			send_sig(SIGKILL, *p, 1);
// TODO WGJA WIP: 		else {
// TODO WGJA WIP: 			for (i=0; i < NR_OPEN; i++) {
// TODO WGJA WIP: 				filp = (*p)->filp[i];
// TODO WGJA WIP: 				if (filp && (filp->f_op == &tty_fops) &&
// TODO WGJA WIP: 				    (MINOR(filp->f_rdev) == line)) {
// TODO WGJA WIP: 					send_sig(SIGKILL, *p, 1);
// TODO WGJA WIP: 					break;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This routine allows a kernel routine to send a large chunk of data
// TODO WGJA WIP:  * to a particular tty; if all of the data can be queued up for ouput
// TODO WGJA WIP:  * immediately, tty_write_data() will return 0.  If, however, not all
// TODO WGJA WIP:  * of the data can be immediately queued for delivery, the number of
// TODO WGJA WIP:  * bytes left to be queued up will be returned, and the rest of the
// TODO WGJA WIP:  * data will be queued up when there is room.  The callback function
// TODO WGJA WIP:  * will be called (with the argument callarg) when the last of the
// TODO WGJA WIP:  * data is finally in the queue.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Note that the callback routine will _not_ be called if all of the
// TODO WGJA WIP:  * data could be queued immediately.  This is to avoid a problem with
// TODO WGJA WIP:  * the kernel stack getting too deep, which might happen if the
// TODO WGJA WIP:  * callback routine calls tty_write_data with itself as an argument.
// TODO WGJA WIP:  */
// TODO WGJA WIP: int tty_write_data(struct tty_struct *tty, char *bufp, int buflen,
// TODO WGJA WIP: 		    void (*callback)(void * data), void * callarg)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int head, tail, count;
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 	char *p;
// TODO WGJA WIP: 
// TODO WGJA WIP: #define VLEFT ((tail-head-1)&(TTY_BUF_SIZE-1))
// TODO WGJA WIP: 
// TODO WGJA WIP: 	save_flags(flags);
// TODO WGJA WIP: 	cli();
// TODO WGJA WIP: 	if (tty->write_data_cnt) {
// TODO WGJA WIP: 		restore_flags(flags);
// TODO WGJA WIP: 		return -EBUSY;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	head = tty->write_q.head;
// TODO WGJA WIP: 	tail = tty->write_q.tail;
// TODO WGJA WIP: 	count = buflen;
// TODO WGJA WIP: 	p = bufp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	while (count && VLEFT > 0) {
// TODO WGJA WIP: 		tty->write_q.buf[head++] = *p++;
// TODO WGJA WIP: 		head &= TTY_BUF_SIZE-1;
// TODO WGJA WIP: 		count--;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	tty->write_q.head = head;
// TODO WGJA WIP: 	if (count) {
// TODO WGJA WIP: 		tty->write_data_cnt = count;
// TODO WGJA WIP: 		tty->write_data_ptr = (unsigned char *) p;
// TODO WGJA WIP: 		tty->write_data_callback = callback;
// TODO WGJA WIP: 		tty->write_data_arg = callarg;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	restore_flags(flags);
// TODO WGJA WIP: 	tty->write(tty);
// TODO WGJA WIP: 	return count;
// TODO WGJA WIP: }

/*
 * This routine routine is called after an interrupt has drained a
 * tty's write queue, so that there is more space for data waiting to
 * be sent in tty->write_data_ptr.
 *
 * tty_check_write[8] is a bitstring which indicates which ttys
 * needs to be processed.
 */
void tty_bh_routine(void * unused)
{
	// TODO WGJA tty_bh_routine
	printk("TODO: tty_bh_routine\n"); for (;;);
// TODO WGJA WIP: 	int	i, j, line, mask;
// TODO WGJA WIP: 	int	head, tail, count;
// TODO WGJA WIP: 	unsigned char * p;
// TODO WGJA WIP: 	struct tty_struct * tty;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for (i = 0, line = 0; i < MAX_TTYS / 32; i++) {
// TODO WGJA WIP: 		if (!tty_check_write[i]) {
// TODO WGJA WIP: 			line += 32;
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		for (j=0, mask=0; j < 32; j++, line++, mask <<= 1) {
// TODO WGJA WIP: 			if (clear_bit(j, &tty_check_write[i])) {
// TODO WGJA WIP: 				tty = tty_table[line];
// TODO WGJA WIP: 				if (!tty || !tty->write_data_cnt)
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				cli();
// TODO WGJA WIP: 				head = tty->write_q.head;
// TODO WGJA WIP: 				tail = tty->write_q.tail;
// TODO WGJA WIP: 				count = tty->write_data_cnt;
// TODO WGJA WIP: 				p = tty->write_data_ptr;
// TODO WGJA WIP: 
// TODO WGJA WIP: 				while (count && VLEFT > 0) {
// TODO WGJA WIP: 					tty->write_q.buf[head++] = *p++;
// TODO WGJA WIP: 					head &= TTY_BUF_SIZE-1;
// TODO WGJA WIP: 					count--;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				tty->write_q.head = head;
// TODO WGJA WIP: 				tty->write_data_ptr = p;
// TODO WGJA WIP: 				tty->write_data_cnt = count;
// TODO WGJA WIP: 				sti();
// TODO WGJA WIP: 				if (!count)
// TODO WGJA WIP: 					(tty->write_data_callback)
// TODO WGJA WIP: 						(tty->write_data_arg);
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
	
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This subroutine initializes a tty structure.  We have to set up
// TODO WGJA WIP:  * things correctly for each different type of tty.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static void initialize_tty_struct(int line, struct tty_struct *tty)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	memset(tty, 0, sizeof(struct tty_struct));
// TODO WGJA WIP: 	tty->line = line;
// TODO WGJA WIP: 	tty->disc = N_TTY;
// TODO WGJA WIP: 	tty->pgrp = -1;
// TODO WGJA WIP: 	tty->winsize.ws_row = 0;
// TODO WGJA WIP: 	tty->winsize.ws_col = 0;
// TODO WGJA WIP: 	if (IS_A_CONSOLE(line)) {
// TODO WGJA WIP: 		tty->open = con_open;
// TODO WGJA WIP: 		tty->winsize.ws_row = video_num_lines;
// TODO WGJA WIP: 		tty->winsize.ws_col = video_num_columns;
// TODO WGJA WIP: 	} else if IS_A_SERIAL(line) {
// TODO WGJA WIP: 		tty->open = rs_open;
// TODO WGJA WIP: 	} else if IS_A_PTY(line) {
// TODO WGJA WIP: 		tty->open = pty_open;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static void initialize_termios(int line, struct termios * tp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	memset(tp, 0, sizeof(struct termios));
// TODO WGJA WIP: 	memcpy(tp->c_cc, INIT_C_CC, NCCS);
// TODO WGJA WIP: 	if (IS_A_CONSOLE(line)) {
// TODO WGJA WIP: 		tp->c_iflag = ICRNL | IXON;
// TODO WGJA WIP: 		tp->c_oflag = OPOST | ONLCR;
// TODO WGJA WIP: 		tp->c_cflag = B38400 | CS8 | CREAD;
// TODO WGJA WIP: 		tp->c_lflag = ISIG | ICANON | ECHO |
// TODO WGJA WIP: 			ECHOCTL | ECHOKE;
// TODO WGJA WIP: 	} else if (IS_A_SERIAL(line)) {
// TODO WGJA WIP: 		tp->c_cflag = B2400 | CS8 | CREAD | HUPCL | CLOCAL;
// TODO WGJA WIP: 		tp->c_oflag = OPOST | ONLCR | XTABS;
// TODO WGJA WIP: 	} else if (IS_A_PTY_MASTER(line)) {
// TODO WGJA WIP: 		tp->c_cflag = B9600 | CS8 | CREAD;
// TODO WGJA WIP: 	} else if (IS_A_PTY_SLAVE(line)) {
// TODO WGJA WIP: 		tp->c_iflag = ICRNL | IXON;
// TODO WGJA WIP: 		tp->c_oflag = OPOST | ONLCR;
// TODO WGJA WIP: 		tp->c_cflag = B38400 | CS8 | CREAD;
// TODO WGJA WIP: 		tp->c_lflag = ISIG | ICANON | ECHO |
// TODO WGJA WIP: 			ECHOCTL | ECHOKE;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }

static struct tty_ldisc tty_ldisc_N_TTY = {
	NULL, // TODO WGJA tty_ldisc:	0,			/* flags */
	NULL, // TODO WGJA tty_ldisc:	NULL,			/* open */
	NULL, // TODO WGJA tty_ldisc:	NULL,			/* close */
	NULL, // TODO WGJA tty_ldisc:	read_chan,		/* read */
	NULL, // TODO WGJA tty_ldisc:	write_chan,		/* write */
	NULL, // TODO WGJA tty_ldisc:	NULL,			/* ioctl */
	NULL, // TODO WGJA tty_ldisc:	copy_to_cooked		/* handler */
};


long tty_init(long kmem_start)
{
	int i;

	if (sizeof(struct tty_struct) > PAGE_SIZE)
		panic("size of tty structure > PAGE_SIZE!");
	if (register_chrdev(4,"tty",&tty_fops))
		panic("unable to get major 4 for tty device");
	if (register_chrdev(5,"tty",&tty_fops))
		panic("unable to get major 5 for tty device");
	for (i=0 ; i< MAX_TTYS ; i++) {
		tty_table[i] =  0;
		tty_termios[i] = 0;
	}
	memset(tty_check_write, 0, sizeof(tty_check_write));
	bh_base[TTY_BH].routine = tty_bh_routine;

	/* Setup the default TTY line discipline. */
	memset(ldiscs, 0, sizeof(ldiscs));
	(void) tty_register_ldisc(N_TTY, &tty_ldisc_N_TTY);

// TODO WGJA WIP: 	kmem_start = kbd_init(kmem_start);
// TODO WGJA WIP: 	kmem_start = con_init(kmem_start);
// TODO WGJA WIP: 	kmem_start = rs_init(kmem_start);
	return kmem_start;
}
