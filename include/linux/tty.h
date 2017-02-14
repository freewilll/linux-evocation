#ifndef _LINUX_TTY_H
#define _LINUX_TTY_H

/*
 * 'tty.h' defines some structures used by tty_io.c and some defines.
 */

// TODO WGJA WIP: #include <linux/termios.h>

#include <asm/system.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: #define NR_CONSOLES	8
// TODO WGJA WIP: #define NR_LDISCS	16
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * These are set up by the setup-routine at boot-time:
// TODO WGJA WIP:  */

struct screen_info {
	unsigned char  orig_x;
	unsigned char  orig_y;
	unsigned char  unused1[2];
	unsigned short orig_video_page;
	unsigned char  orig_video_mode;
	unsigned char  orig_video_cols;
	unsigned short orig_video_ega_ax;
	unsigned short orig_video_ega_bx;
	unsigned short orig_video_ega_cx;
	unsigned char  orig_video_lines;
};

extern struct screen_info screen_info;

// TODO WGJA WIP: #define ORIG_X			(screen_info.orig_x)
// TODO WGJA WIP: #define ORIG_Y			(screen_info.orig_y)
// TODO WGJA WIP: #define ORIG_VIDEO_PAGE		(screen_info.orig_video_page)
// TODO WGJA WIP: #define ORIG_VIDEO_MODE		(screen_info.orig_video_mode)
// TODO WGJA WIP: #define ORIG_VIDEO_COLS 	(screen_info.orig_video_cols)
// TODO WGJA WIP: #define ORIG_VIDEO_EGA_AX	(screen_info.orig_video_ega_ax)
// TODO WGJA WIP: #define ORIG_VIDEO_EGA_BX	(screen_info.orig_video_ega_bx)
// TODO WGJA WIP: #define ORIG_VIDEO_EGA_CX	(screen_info.orig_video_ega_cx)
// TODO WGJA WIP: #define ORIG_VIDEO_LINES	(screen_info.orig_video_lines)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define VIDEO_TYPE_MDA		0x10	/* Monochrome Text Display	*/
// TODO WGJA WIP: #define VIDEO_TYPE_CGA		0x11	/* CGA Display 			*/
// TODO WGJA WIP: #define VIDEO_TYPE_EGAM		0x20	/* EGA/VGA in Monochrome Mode	*/
// TODO WGJA WIP: #define VIDEO_TYPE_EGAC		0x21	/* EGA/VGA in Color Mode	*/
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This character is the same as _POSIX_VDISABLE: it cannot be used as
// TODO WGJA WIP:  * a c_cc[] character, but indicates that a particular special character
// TODO WGJA WIP:  * isn't in use (eg VINTR ahs no character etc)
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define __DISABLED_CHAR '\0'
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * See comment for the tty_struct structure before changing
// TODO WGJA WIP:  * TTY_BUF_SIZE.  Actually, there should be different sized tty_queue
// TODO WGJA WIP:  * structures for different purposes.  1024 bytes for the transmit
// TODO WGJA WIP:  * queue is way overkill.  TYT, 9/14/92
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define TTY_BUF_SIZE 1024	/* Must be a power of 2 */
// TODO WGJA WIP: 
// TODO WGJA WIP: struct tty_queue {
// TODO WGJA WIP: 	unsigned long data;
// TODO WGJA WIP: 	unsigned long head;
// TODO WGJA WIP: 	unsigned long tail;
// TODO WGJA WIP: 	struct wait_queue * proc_list;
// TODO WGJA WIP: 	unsigned char buf[TTY_BUF_SIZE];
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: struct serial_struct {
// TODO WGJA WIP: 	int	type;
// TODO WGJA WIP: 	int	line;
// TODO WGJA WIP: 	int	port;
// TODO WGJA WIP: 	int	irq;
// TODO WGJA WIP: 	int	flags;
// TODO WGJA WIP: 	int	xmit_fifo_size;
// TODO WGJA WIP: 	int	custom_divisor;
// TODO WGJA WIP: 	int	baud_base;
// TODO WGJA WIP: 	char	close_delay;
// TODO WGJA WIP: 	char	reserved_char[3];
// TODO WGJA WIP: 	int	hub6;
// TODO WGJA WIP: 	int	reserved[5];
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * These are the supported serial types.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define PORT_UNKNOWN	0
// TODO WGJA WIP: #define PORT_8250	1
// TODO WGJA WIP: #define PORT_16450	2
// TODO WGJA WIP: #define PORT_16550	3
// TODO WGJA WIP: #define PORT_16550A	4
// TODO WGJA WIP: #define PORT_MAX	4
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Definitions for async_struct (and serial_struct) flags field
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define ASYNC_HUP_NOTIFY 0x0001 /* Notify blocked open on hangups */
// TODO WGJA WIP: #define ASYNC_FOURPORT  0x0002	/* Set OU1, OUT2 per AST Fourport settings */
// TODO WGJA WIP: #define ASYNC_SAK	0x0004	/* Secure Attention Key (Orange book) */
// TODO WGJA WIP: #define ASYNC_TERMIOS_RESTORE 0x0008 /* Restore termios when dialin unblocks */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define ASYNC_SPD_MASK	0x0030
// TODO WGJA WIP: #define ASYNC_SPD_HI	0x0010	/* Use 56000 instead of 38400 bps */
// TODO WGJA WIP: #define ASYNC_SPD_VHI	0x0020  /* Use 115200 instead of 38400 bps */
// TODO WGJA WIP: #define ASYNC_SPD_CUST	0x0030  /* Use user-specified divisor */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define ASYNC_SKIP_TEST	0x0040 /* Skip UART test during autoconfiguration */
// TODO WGJA WIP: #define ASYNC_AUTO_IRQ  0x0080 /* Do automatic IRQ during autoconfiguration */
// TODO WGJA WIP: #define ASYNC_SESSION_LOCKOUT 0x0100 /* Lock out cua opens based on session */
// TODO WGJA WIP: #define ASYNC_PGRP_LOCKOUT    0x0200 /* Lock out cua opens based on pgrp */
// TODO WGJA WIP: #define ASYNC_CALLOUT_NOHUP   0x0400 /* Don't do hangups for cua device */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define ASYNC_FLAGS	0x0FFF	/* Possible legal async flags */
// TODO WGJA WIP: #define ASYNC_USR_MASK 0x0430	/* Legal flags that non-privileged
// TODO WGJA WIP: 				 * users can set or reset */
// TODO WGJA WIP: 
// TODO WGJA WIP: /* Internal flags used only by kernel/chr_drv/serial.c */
// TODO WGJA WIP: #define ASYNC_INITIALIZED	0x80000000 /* Serial port was initialized */
// TODO WGJA WIP: #define ASYNC_CALLOUT_ACTIVE	0x40000000 /* Call out device is active */
// TODO WGJA WIP: #define ASYNC_NORMAL_ACTIVE	0x20000000 /* Normal device is active */
// TODO WGJA WIP: #define ASYNC_BOOT_AUTOCONF	0x10000000 /* Autoconfigure port on bootup */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define IS_A_CONSOLE(min)	(((min) & 0xC0) == 0x00)
// TODO WGJA WIP: #define IS_A_SERIAL(min)	(((min) & 0xC0) == 0x40)
// TODO WGJA WIP: #define IS_A_PTY(min)		((min) & 0x80)
// TODO WGJA WIP: #define IS_A_PTY_MASTER(min)	(((min) & 0xC0) == 0x80)
// TODO WGJA WIP: #define IS_A_PTY_SLAVE(min)	(((min) & 0xC0) == 0xC0)
// TODO WGJA WIP: #define PTY_OTHER(min)		((min) ^ 0x40)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define SL_TO_DEV(line)		((line) | 0x40)
// TODO WGJA WIP: #define DEV_TO_SL(min)		((min) & 0x3F)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define INC(a) ((a) = ((a)+1) & (TTY_BUF_SIZE-1))
// TODO WGJA WIP: #define DEC(a) ((a) = ((a)-1) & (TTY_BUF_SIZE-1))
// TODO WGJA WIP: #define EMPTY(a) ((a)->head == (a)->tail)
// TODO WGJA WIP: #define LEFT(a) (((a)->tail-(a)->head-1)&(TTY_BUF_SIZE-1))
// TODO WGJA WIP: #define LAST(a) ((a)->buf[(TTY_BUF_SIZE-1)&((a)->head-1)])
// TODO WGJA WIP: #define FULL(a) (!LEFT(a))
// TODO WGJA WIP: #define CHARS(a) (((a)->head-(a)->tail)&(TTY_BUF_SIZE-1))
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void put_tty_queue(char c, struct tty_queue * queue);
// TODO WGJA WIP: extern int get_tty_queue(struct tty_queue * queue);
// TODO WGJA WIP: 
// TODO WGJA WIP: #define INTR_CHAR(tty) ((tty)->termios->c_cc[VINTR])
// TODO WGJA WIP: #define QUIT_CHAR(tty) ((tty)->termios->c_cc[VQUIT])
// TODO WGJA WIP: #define ERASE_CHAR(tty) ((tty)->termios->c_cc[VERASE])
// TODO WGJA WIP: #define KILL_CHAR(tty) ((tty)->termios->c_cc[VKILL])
// TODO WGJA WIP: #define WERASE_CHAR(tty) ((tty)->termios->c_cc[VWERASE])
// TODO WGJA WIP: #define EOF_CHAR(tty) ((tty)->termios->c_cc[VEOF])
// TODO WGJA WIP: #define START_CHAR(tty) ((tty)->termios->c_cc[VSTART])
// TODO WGJA WIP: #define STOP_CHAR(tty) ((tty)->termios->c_cc[VSTOP])
// TODO WGJA WIP: #define SUSPEND_CHAR(tty) ((tty)->termios->c_cc[VSUSP])
// TODO WGJA WIP: #define LNEXT_CHAR(tty)	((tty)->termios->c_cc[VLNEXT])
// TODO WGJA WIP: 
// TODO WGJA WIP: #define _L_FLAG(tty,f)	((tty)->termios->c_lflag & f)
// TODO WGJA WIP: #define _I_FLAG(tty,f)	((tty)->termios->c_iflag & f)
// TODO WGJA WIP: #define _O_FLAG(tty,f)	((tty)->termios->c_oflag & f)
// TODO WGJA WIP: #define _C_FLAG(tty,f)	((tty)->termios->c_cflag & f)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define L_CANON(tty)	_L_FLAG((tty),ICANON)
// TODO WGJA WIP: #define L_ISIG(tty)	_L_FLAG((tty),ISIG)
// TODO WGJA WIP: #define L_ECHO(tty)	_L_FLAG((tty),ECHO)
// TODO WGJA WIP: #define L_ECHOE(tty)	_L_FLAG((tty),ECHOE)
// TODO WGJA WIP: #define L_ECHOK(tty)	_L_FLAG((tty),ECHOK)
// TODO WGJA WIP: #define L_ECHONL(tty)	_L_FLAG((tty),ECHONL)
// TODO WGJA WIP: #define L_ECHOCTL(tty)	_L_FLAG((tty),ECHOCTL)
// TODO WGJA WIP: #define L_ECHOKE(tty)	_L_FLAG((tty),ECHOKE)
// TODO WGJA WIP: #define L_TOSTOP(tty)	_L_FLAG((tty),TOSTOP)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define I_IGNBRK(tty)	_I_FLAG((tty),IGNBRK)
// TODO WGJA WIP: #define I_BRKINT(tty)	_I_FLAG((tty),BRKINT)
// TODO WGJA WIP: #define I_IGNPAR(tty)	_I_FLAG((tty),IGNPAR)
// TODO WGJA WIP: #define I_PARMRK(tty)	_I_FLAG((tty),PARMRK)
// TODO WGJA WIP: #define I_INPCK(tty)	_I_FLAG((tty),INPCK)
// TODO WGJA WIP: #define I_UCLC(tty)	_I_FLAG((tty),IUCLC)
// TODO WGJA WIP: #define I_NLCR(tty)	_I_FLAG((tty),INLCR)
// TODO WGJA WIP: #define I_CRNL(tty)	_I_FLAG((tty),ICRNL)
// TODO WGJA WIP: #define I_NOCR(tty)	_I_FLAG((tty),IGNCR)
// TODO WGJA WIP: #define I_IXON(tty)	_I_FLAG((tty),IXON)
// TODO WGJA WIP: #define I_IXANY(tty)	_I_FLAG((tty),IXANY)
// TODO WGJA WIP: #define I_STRP(tty)	_I_FLAG((tty),ISTRIP)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define O_POST(tty)	_O_FLAG((tty),OPOST)
// TODO WGJA WIP: #define O_LCUC(tty)	_O_FLAG((tty),OLCUC)
// TODO WGJA WIP: #define O_NLCR(tty)	_O_FLAG((tty),ONLCR)
// TODO WGJA WIP: #define O_CRNL(tty)	_O_FLAG((tty),OCRNL)
// TODO WGJA WIP: #define O_NOCR(tty)	_O_FLAG((tty),ONOCR)
// TODO WGJA WIP: #define O_NLRET(tty)	_O_FLAG((tty),ONLRET)
// TODO WGJA WIP: #define O_TABDLY(tty)	_O_FLAG((tty),TABDLY)
// TODO WGJA WIP: 
// TODO WGJA WIP: #define C_LOCAL(tty)	_C_FLAG((tty),CLOCAL)
// TODO WGJA WIP: #define C_RTSCTS(tty)	_C_FLAG((tty),CRTSCTS)
// TODO WGJA WIP: #define C_SPEED(tty)	((tty)->termios->c_cflag & CBAUD)
// TODO WGJA WIP: #define C_HUP(tty)	(C_SPEED((tty)) == B0)
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Where all of the state associated with a tty is kept while the tty
// TODO WGJA WIP:  * is open.  Since the termios state should be kept even if the tty
// TODO WGJA WIP:  * has been closed --- for things like the baud rate, etc --- it is
// TODO WGJA WIP:  * not stored here, but rather a pointer to the real state is stored
// TODO WGJA WIP:  * here.  Possible the winsize structure should have the same
// TODO WGJA WIP:  * treatment, but (1) the default 80x24 is usually right and (2) it's
// TODO WGJA WIP:  * most often used by a windowing system, which will set the correct
// TODO WGJA WIP:  * size each time the window is created or resized anyway.
// TODO WGJA WIP:  * IMPORTANT: since this structure is dynamically allocated, it must
// TODO WGJA WIP:  * be no larger than 4096 bytes.  Changing TTY_BUF_SIZE will change
// TODO WGJA WIP:  * the size of this structure, and it needs to be done with care.
// TODO WGJA WIP:  * 						- TYT, 9/14/92
// TODO WGJA WIP:  */
// TODO WGJA WIP: struct tty_struct {
// TODO WGJA WIP: 	struct termios *termios;
// TODO WGJA WIP: 	int pgrp;
// TODO WGJA WIP: 	int session;
// TODO WGJA WIP: 	unsigned char stopped:1, status_changed:1, packet:1, lnext:1;
// TODO WGJA WIP: 	unsigned char char_error:2;
// TODO WGJA WIP: 	unsigned char ctrl_status;
// TODO WGJA WIP: 	short line;
// TODO WGJA WIP: 	int disc;
// TODO WGJA WIP: 	int flags;
// TODO WGJA WIP: 	int count;
// TODO WGJA WIP: 	int column;
// TODO WGJA WIP: 	struct winsize winsize;
// TODO WGJA WIP: 	int  (*open)(struct tty_struct * tty, struct file * filp);
// TODO WGJA WIP: 	void (*close)(struct tty_struct * tty, struct file * filp);
// TODO WGJA WIP: 	void (*write)(struct tty_struct * tty);
// TODO WGJA WIP: 	int  (*ioctl)(struct tty_struct *tty, struct file * file,
// TODO WGJA WIP: 		    unsigned int cmd, unsigned long arg);
// TODO WGJA WIP: 	void (*throttle)(struct tty_struct * tty, int status);
// TODO WGJA WIP: 	void (*set_termios)(struct tty_struct *tty, struct termios * old);
// TODO WGJA WIP: 	void (*stop)(struct tty_struct *tty);
// TODO WGJA WIP: 	void (*start)(struct tty_struct *tty);
// TODO WGJA WIP: 	struct tty_struct *link;
// TODO WGJA WIP: 	unsigned char *write_data_ptr;
// TODO WGJA WIP: 	int write_data_cnt;
// TODO WGJA WIP: 	void (*write_data_callback)(void * data);
// TODO WGJA WIP: 	void * write_data_arg;
// TODO WGJA WIP: 	int readq_flags[TTY_BUF_SIZE/32];
// TODO WGJA WIP: 	struct tty_queue read_q;
// TODO WGJA WIP: 	struct tty_queue write_q;
// TODO WGJA WIP: 	struct tty_queue secondary;
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: struct tty_ldisc {
// TODO WGJA WIP: 	int	flags;
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * The following routines are called from above.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	int	(*open)(struct tty_struct *);
// TODO WGJA WIP: 	void	(*close)(struct tty_struct *);
// TODO WGJA WIP: 	int	(*read)(struct tty_struct * tty, struct file * file,
// TODO WGJA WIP: 			char * buf, int nr);
// TODO WGJA WIP: 	int	(*write)(struct tty_struct * tty, struct file * file,
// TODO WGJA WIP: 			 char * buf, int nr);	
// TODO WGJA WIP: 	int	(*ioctl)(struct tty_struct * tty, struct file * file,
// TODO WGJA WIP: 			 unsigned int cmd, unsigned long arg);
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * The following routines are called from below.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	void	(*handler)(struct tty_struct *);
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: #define LDISC_FLAG_DEFINED	0x00000001
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * These are the different types of thottle status which can be sent
// TODO WGJA WIP:  * to the low-level tty driver.  The tty_io.c layer is responsible for
// TODO WGJA WIP:  * notifying the low-level tty driver of the following conditions:
// TODO WGJA WIP:  * secondary queue full, secondary queue available, and read queue
// TODO WGJA WIP:  * available.  The low-level driver must send the read queue full
// TODO WGJA WIP:  * command to itself, if it is interested in that condition.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Note that the low-level tty driver may elect to ignore one or both
// TODO WGJA WIP:  * of these conditions; normally, however, it will use ^S/^Q or some
// TODO WGJA WIP:  * sort of hardware flow control to regulate the input to try to avoid
// TODO WGJA WIP:  * overflow.  While the low-level driver is responsible for all
// TODO WGJA WIP:  * receiving flow control, note that the ^S/^Q handling (but not
// TODO WGJA WIP:  * hardware flow control) is handled by the upper layer, in
// TODO WGJA WIP:  * copy_to_cooked.  
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define TTY_THROTTLE_SQ_FULL	1
// TODO WGJA WIP: #define TTY_THROTTLE_SQ_AVAIL	2
// TODO WGJA WIP: #define TTY_THROTTLE_RQ_FULL	3
// TODO WGJA WIP: #define TTY_THROTTLE_RQ_AVAIL	4
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This defines the low- and high-watermarks for the various conditions.
// TODO WGJA WIP:  * Again, the low-level driver is free to ignore any of these, and has
// TODO WGJA WIP:  * to implement RQ_THREHOLD_LW for itself if it wants it.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define SQ_THRESHOLD_LW	16
// TODO WGJA WIP: #define SQ_THRESHOLD_HW 768
// TODO WGJA WIP: #define RQ_THRESHOLD_LW 16
// TODO WGJA WIP: #define RQ_THRESHOLD_HW 768
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * These bits are used in the flags field of the tty structure.
// TODO WGJA WIP:  * 
// TODO WGJA WIP:  * So that interrupts won't be able to mess up the queues,
// TODO WGJA WIP:  * copy_to_cooked must be atomic with repect to itself, as must
// TODO WGJA WIP:  * tty->write.  Thus, you must use the inline functions set_bit() and
// TODO WGJA WIP:  * clear_bit() to make things atomic.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define TTY_WRITE_BUSY 0
// TODO WGJA WIP: #define TTY_READ_BUSY 1
// TODO WGJA WIP: #define TTY_CR_PENDING 2
// TODO WGJA WIP: #define TTY_SQ_THROTTLED 3
// TODO WGJA WIP: #define TTY_RQ_THROTTLED 4
// TODO WGJA WIP: #define TTY_IO_ERROR 5
// TODO WGJA WIP: #define TTY_SLAVE_OPENED 6
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * When a break, frame error, or parity error happens, these codes are
// TODO WGJA WIP:  * stuffed into the read queue, and the relevant bit in readq_flag bit
// TODO WGJA WIP:  * array is set.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define TTY_BREAK	1
// TODO WGJA WIP: #define TTY_FRAME	2
// TODO WGJA WIP: #define TTY_PARITY	3
// TODO WGJA WIP: 
// TODO WGJA WIP: #define TTY_WRITE_FLUSH(tty) tty_write_flush((tty))
// TODO WGJA WIP: #define TTY_READ_FLUSH(tty) tty_read_flush((tty))
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void tty_write_flush(struct tty_struct *);
// TODO WGJA WIP: extern void tty_read_flush(struct tty_struct *);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern struct tty_struct *tty_table[];
// TODO WGJA WIP: extern struct termios *tty_termios[];
// TODO WGJA WIP: extern struct termios *termios_locked[];
// TODO WGJA WIP: extern int tty_check_write[];
// TODO WGJA WIP: extern struct tty_struct * redirect;
// TODO WGJA WIP: extern struct tty_ldisc ldiscs[];
// TODO WGJA WIP: extern int fg_console;
// TODO WGJA WIP: extern unsigned long video_num_columns;
// TODO WGJA WIP: extern unsigned long video_num_lines;
// TODO WGJA WIP: extern struct wait_queue * keypress_wait;
// TODO WGJA WIP: 
// TODO WGJA WIP: #define TTY_TABLE_IDX(nr)	((nr) ? (nr) : (fg_console+1))
// TODO WGJA WIP: #define TTY_TABLE(nr) 		(tty_table[TTY_TABLE_IDX(nr)])
// TODO WGJA WIP: 
// TODO WGJA WIP: /*	intr=^C		quit=^|		erase=del	kill=^U
// TODO WGJA WIP: 	eof=^D		vtime=\0	vmin=\1		sxtc=\0
// TODO WGJA WIP: 	start=^Q	stop=^S		susp=^Z		eol=\0
// TODO WGJA WIP: 	reprint=^R	discard=^U	werase=^W	lnext=^V
// TODO WGJA WIP: 	eol2=\0
// TODO WGJA WIP: */
// TODO WGJA WIP: #define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"
// TODO WGJA WIP: 
// TODO WGJA WIP: extern long rs_init(long);
// TODO WGJA WIP: extern long lp_init(long);
// TODO WGJA WIP: extern long con_init(long);
// TODO WGJA WIP: extern long tty_init(long);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void flush_input(struct tty_struct * tty);
// TODO WGJA WIP: extern void flush_output(struct tty_struct * tty);
// TODO WGJA WIP: extern void wait_until_sent(struct tty_struct * tty);
// TODO WGJA WIP: extern void copy_to_cooked(struct tty_struct * tty);
// TODO WGJA WIP: extern int tty_register_ldisc(int disc, struct tty_ldisc *new_ldisc);
// TODO WGJA WIP: extern int tty_read_raw_data(struct tty_struct *tty, unsigned char *bufp,
// TODO WGJA WIP: 			     int buflen);
// TODO WGJA WIP: extern int tty_write_data(struct tty_struct *tty, char *bufp, int buflen,
// TODO WGJA WIP: 			  void (*callback)(void * data), void * callarg);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int tty_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
// TODO WGJA WIP: extern int is_orphaned_pgrp(int pgrp);
// TODO WGJA WIP: extern int is_ignored(int sig);
// TODO WGJA WIP: extern int tty_signal(int sig, struct tty_struct *tty);
// TODO WGJA WIP: extern int kill_pg(int pgrp, int sig, int priv);
// TODO WGJA WIP: extern int kill_sl(int sess, int sig, int priv);
// TODO WGJA WIP: extern void tty_hangup(struct tty_struct * tty);
// TODO WGJA WIP: extern void tty_vhangup(struct tty_struct * tty);
// TODO WGJA WIP: extern void tty_unhangup(struct file *filp);
// TODO WGJA WIP: extern int tty_hung_up_p(struct file * filp);
// TODO WGJA WIP: extern void do_SAK(struct tty_struct *tty);
// TODO WGJA WIP: 
// TODO WGJA WIP: /* tty write functions */
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void rs_write(struct tty_struct * tty);
// TODO WGJA WIP: extern void con_write(struct tty_struct * tty);
// TODO WGJA WIP: 
// TODO WGJA WIP: /* serial.c */
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int  rs_open(struct tty_struct * tty, struct file * filp);
// TODO WGJA WIP: 
// TODO WGJA WIP: /* pty.c */
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int  pty_open(struct tty_struct * tty, struct file * filp);
// TODO WGJA WIP: 
// TODO WGJA WIP: /* console.c */
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int con_open(struct tty_struct * tty, struct file * filp);
// TODO WGJA WIP: extern void update_screen(int new_console);
// TODO WGJA WIP: extern void blank_screen(void);
// TODO WGJA WIP: extern void unblank_screen(void);
// TODO WGJA WIP: 
// TODO WGJA WIP: /* vt.c */
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int vt_ioctl(struct tty_struct *tty, struct file * file,
// TODO WGJA WIP: 		    unsigned int cmd, unsigned long arg);

#endif
