// TODO WGJA WIP: /*
// TODO WGJA WIP:  *  linux/init/main.c
// TODO WGJA WIP:  *
// TODO WGJA WIP:  *  Copyright (C) 1991, 1992  Linus Torvalds
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: #include <stdarg.h>
// TODO WGJA WIP: 
#include <asm/system.h>
// TODO WGJA WIP: #include <asm/io.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: #include <linux/mktime.h>
// TODO WGJA WIP: #include <linux/types.h>
// TODO WGJA WIP: #include <linux/fcntl.h>
// TODO WGJA WIP: #include <linux/config.h>
// TODO WGJA WIP: #include <linux/sched.h>
// TODO WGJA WIP: #include <linux/tty.h>
#include <linux/head.h>
// TODO WGJA WIP: #include <linux/unistd.h>
// TODO WGJA WIP: #include <linux/string.h>
// TODO WGJA WIP: #include <linux/timer.h>
// TODO WGJA WIP: #include <linux/fs.h>
// TODO WGJA WIP: #include <linux/ctype.h>
// TODO WGJA WIP: #include <linux/delay.h>
// TODO WGJA WIP: #include <linux/utsname.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: extern unsigned long * prof_buffer;
// TODO WGJA WIP: extern unsigned long prof_len;
// TODO WGJA WIP: extern char edata, end;
// TODO WGJA WIP: extern char *linux_banner;
extern "C" void lcall7(void);
struct desc_struct default_ldt;
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * we need this inline - forking from kernel space will result
// TODO WGJA WIP:  * in NO COPY ON WRITE (!!!), until an execve is executed. This
// TODO WGJA WIP:  * is no problem, but for the stack. This is handled by not letting
// TODO WGJA WIP:  * main() use the stack at all after fork(). Thus, no function
// TODO WGJA WIP:  * calls - which means inline code for fork too, as otherwise we
// TODO WGJA WIP:  * would use the stack upon exit from 'fork()'.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Actually only pause and fork are needed inline, so that there
// TODO WGJA WIP:  * won't be any messing with the stack from main(), but we define
// TODO WGJA WIP:  * some others too.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static inline _syscall0(int,idle)
// TODO WGJA WIP: static inline _syscall0(int,fork)
// TODO WGJA WIP: static inline _syscall0(int,pause)
// TODO WGJA WIP: static inline _syscall1(int,setup,void *,BIOS)
// TODO WGJA WIP: static inline _syscall0(int,sync)
// TODO WGJA WIP: static inline _syscall0(pid_t,setsid)
// TODO WGJA WIP: static inline _syscall3(int,write,int,fd,const char *,buf,off_t,count)
// TODO WGJA WIP: static inline _syscall1(int,dup,int,fd)
// TODO WGJA WIP: static inline _syscall3(int,execve,const char *,file,char **,argv,char **,envp)
// TODO WGJA WIP: static inline _syscall3(int,open,const char *,file,int,flag,int,mode)
// TODO WGJA WIP: static inline _syscall1(int,close,int,fd)
// TODO WGJA WIP: static inline _syscall1(int,exit,int,exitcode)
// TODO WGJA WIP: static inline _syscall3(pid_t,waitpid,pid_t,pid,int *,wait_stat,int,options)
// TODO WGJA WIP: 
// TODO WGJA WIP: static inline pid_t wait(int * wait_stat)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return waitpid(-1,wait_stat,0);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static char printbuf[1024];
// TODO WGJA WIP: 
void init_early_printk();
// TODO WGJA WIP: extern char empty_zero_page[PAGE_SIZE];
// TODO WGJA WIP: extern int vsprintf(char *,const char *,va_list);
// TODO WGJA WIP: extern void init(void);
// TODO WGJA WIP: extern void init_IRQ(void);
// TODO WGJA WIP: extern long blk_dev_init(long,long);
// TODO WGJA WIP: extern long chr_dev_init(long,long);
// TODO WGJA WIP: extern void floppy_init(void);
// TODO WGJA WIP: extern void sock_init(void);
// TODO WGJA WIP: extern long rd_init(long mem_start, int length);
// TODO WGJA WIP: extern long kernel_mktime(struct mktime * time);
// TODO WGJA WIP: extern unsigned long simple_strtoul(const char *,char **,unsigned int);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void hd_setup(char *str, int *ints);
// TODO WGJA WIP: extern void bmouse_setup(char *str, int *ints);
// TODO WGJA WIP: extern void eth_setup(char *str, int *ints);
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifdef CONFIG_SYSVIPC
// TODO WGJA WIP: extern void ipc_init(void);
// TODO WGJA WIP: #endif
// TODO WGJA WIP: #ifdef CONFIG_SCSI
// TODO WGJA WIP: extern unsigned long scsi_dev_init(unsigned long, unsigned long);
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This is set up by the setup-routine at boot-time
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define PARAM	empty_zero_page
// TODO WGJA WIP: #define EXT_MEM_K (*(unsigned short *) (PARAM+2))
// TODO WGJA WIP: #define DRIVE_INFO (*(struct drive_info_struct *) (PARAM+0x80))
// TODO WGJA WIP: #define SCREEN_INFO (*(struct screen_info *) (PARAM+0))
// TODO WGJA WIP: #define MOUNT_ROOT_RDONLY (*(unsigned short *) (PARAM+0x1F2))
// TODO WGJA WIP: #define RAMDISK_SIZE (*(unsigned short *) (PARAM+0x1F8))
// TODO WGJA WIP: #define ORIG_ROOT_DEV (*(unsigned short *) (PARAM+0x1FC))
// TODO WGJA WIP: #define AUX_DEVICE_INFO (*(unsigned char *) (PARAM+0x1FF))
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Boot command-line arguments
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define MAX_INIT_ARGS 8
// TODO WGJA WIP: #define MAX_INIT_ENVS 8
// TODO WGJA WIP: #define COMMAND_LINE ((char *) (PARAM+2048))
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
// TODO WGJA WIP:  * and this seems to work. I anybody has more info on the real-time
// TODO WGJA WIP:  * clock I'd be interested. Most of this was trial and error, and some
// TODO WGJA WIP:  * bios-listing reading. Urghh.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define CMOS_READ(addr) ({ \
// TODO WGJA WIP: outb_p(addr,0x70); \
// TODO WGJA WIP: inb_p(0x71); \
// TODO WGJA WIP: })
// TODO WGJA WIP: 
// TODO WGJA WIP: #define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)
// TODO WGJA WIP: 
// TODO WGJA WIP: static void time_init(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct mktime time;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for (i = 0 ; i < 1000000 ; i++)
// TODO WGJA WIP: 		if (!(CMOS_READ(10) & 0x80))
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 	do {
// TODO WGJA WIP: 		time.sec = CMOS_READ(0);
// TODO WGJA WIP: 		time.min = CMOS_READ(2);
// TODO WGJA WIP: 		time.hour = CMOS_READ(4);
// TODO WGJA WIP: 		time.day = CMOS_READ(7);
// TODO WGJA WIP: 		time.mon = CMOS_READ(8);
// TODO WGJA WIP: 		time.year = CMOS_READ(9);
// TODO WGJA WIP: 	} while (time.sec != CMOS_READ(0));
// TODO WGJA WIP: 	BCD_TO_BIN(time.sec);
// TODO WGJA WIP: 	BCD_TO_BIN(time.min);
// TODO WGJA WIP: 	BCD_TO_BIN(time.hour);
// TODO WGJA WIP: 	BCD_TO_BIN(time.day);
// TODO WGJA WIP: 	BCD_TO_BIN(time.mon);
// TODO WGJA WIP: 	BCD_TO_BIN(time.year);
// TODO WGJA WIP: 	time.mon--;
// TODO WGJA WIP: 	startup_time = kernel_mktime(&time);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static unsigned long memory_start = 0;	/* After mem_init, stores the */
// TODO WGJA WIP: 					/* amount of free user memory */
// TODO WGJA WIP: static unsigned long memory_end = 0;
// TODO WGJA WIP: static unsigned long low_memory_start = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: static char * argv_init[MAX_INIT_ARGS+2] = { "init", NULL, };
// TODO WGJA WIP: static char * envp_init[MAX_INIT_ENVS+2] = { "HOME=/", "TERM=console", NULL, };
// TODO WGJA WIP: 
// TODO WGJA WIP: static char * argv_rc[] = { "/bin/sh", NULL };
// TODO WGJA WIP: static char * envp_rc[] = { "HOME=/", "TERM=console", NULL };
// TODO WGJA WIP: 
// TODO WGJA WIP: static char * argv[] = { "-/bin/sh",NULL };
// TODO WGJA WIP: static char * envp[] = { "HOME=/usr/root", "TERM=console", NULL };
// TODO WGJA WIP: 
// TODO WGJA WIP: struct drive_info_struct { char dummy[32]; } drive_info;
// TODO WGJA WIP: struct screen_info screen_info;
// TODO WGJA WIP: 
// TODO WGJA WIP: unsigned char aux_device_present;
// TODO WGJA WIP: int ramdisk_size;
// TODO WGJA WIP: int root_mountflags = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: static char fpu_error = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: static char command_line[80] = { 0, };
// TODO WGJA WIP: 
// TODO WGJA WIP: char *get_options(char *str, int *ints) 
// TODO WGJA WIP: {
// TODO WGJA WIP: 	char *cur = str;
// TODO WGJA WIP: 	int i=1;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	while (cur && isdigit(*cur) && i <= 10) {
// TODO WGJA WIP: 		ints[i++] = simple_strtoul(cur,NULL,0);
// TODO WGJA WIP: 		if ((cur = strchr(cur,',')) != NULL)
// TODO WGJA WIP: 			cur++;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	ints[0] = i-1;
// TODO WGJA WIP: 	return(cur);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: struct {
// TODO WGJA WIP: 	char *str;
// TODO WGJA WIP: 	void (*setup_func)(char *, int *);
// TODO WGJA WIP: } bootsetups[] = {
// TODO WGJA WIP: #ifdef CONFIG_INET
// TODO WGJA WIP: 	{ "ether=", eth_setup },
// TODO WGJA WIP: #endif
// TODO WGJA WIP: #ifdef CONFIG_BLK_DEV_HD
// TODO WGJA WIP: 	{ "hd=", hd_setup },
// TODO WGJA WIP: #endif
// TODO WGJA WIP: #ifdef CONFIG_BUSMOUSE
// TODO WGJA WIP: 	{ "bmouse=", bmouse_setup },
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	{ 0, 0 }
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: int checksetup(char *line)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i = 0;
// TODO WGJA WIP: 	int ints[11];
// TODO WGJA WIP: 
// TODO WGJA WIP: 	while (bootsetups[i].str) {
// TODO WGJA WIP: 		int n = strlen(bootsetups[i].str);
// TODO WGJA WIP: 		if (!strncmp(line,bootsetups[i].str,n)) {
// TODO WGJA WIP: 			bootsetups[i].setup_func(get_options(line+n,ints), ints);
// TODO WGJA WIP: 			return(0);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		i++;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return(1);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: unsigned long loops_per_sec = 1;
// TODO WGJA WIP: 
// TODO WGJA WIP: static void calibrate_delay(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int ticks;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	printk("Calibrating delay loop.. ");
// TODO WGJA WIP: 	while (loops_per_sec <<= 1) {
// TODO WGJA WIP: 		ticks = jiffies;
// TODO WGJA WIP: 		__delay(loops_per_sec);
// TODO WGJA WIP: 		ticks = jiffies - ticks;
// TODO WGJA WIP: 		if (ticks >= HZ) {
// TODO WGJA WIP: 			__asm__("mull %1 ; divl %2"
// TODO WGJA WIP: 				:"=a" (loops_per_sec)
// TODO WGJA WIP: 				:"d" (HZ),
// TODO WGJA WIP: 				 "r" (ticks),
// TODO WGJA WIP: 				 "0" (loops_per_sec)
// TODO WGJA WIP: 				:"dx");
// TODO WGJA WIP: 			printk("ok - %d.%02d BogoMips (tm)\n",
// TODO WGJA WIP: 				loops_per_sec/500000,
// TODO WGJA WIP: 				(loops_per_sec/5000) % 100);
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	printk("failed\n");
// TODO WGJA WIP: }
	// TODO WGJA WIP: 
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This is a simple kernel command line parsing function: it parses
// TODO WGJA WIP:  * the command line, and fills in the arguments/environment to init
// TODO WGJA WIP:  * as appropriate. Any cmd-line option is taken to be an environment
// TODO WGJA WIP:  * variable if it contains the character '='.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * This routine also checks for options meant for the kernel - currently
// TODO WGJA WIP:  * only the "root=XXXX" option is recognized. These options are not given
// TODO WGJA WIP:  * to init - they are for internal kernel use only.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static void parse_options(char *line)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	char *next;
// TODO WGJA WIP: 	char *devnames[] = { "hda", "hdb", "sda", "sdb", "sdc", "sdd", "sde", "fd", NULL };
// TODO WGJA WIP: 	int devnums[]    = { 0x300, 0x340, 0x800, 0x810, 0x820, 0x830, 0x840, 0x200, 0};
// TODO WGJA WIP: 	int args, envs;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!*line)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	args = 0;
// TODO WGJA WIP: 	envs = 1;	/* TERM is set to 'console' by default */
// TODO WGJA WIP: 	next = line;
// TODO WGJA WIP: 	while ((line = next) != NULL) {
// TODO WGJA WIP: 		if ((next = strchr(line,' ')) != NULL)
// TODO WGJA WIP: 			*next++ = 0;
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * check for kernel options first..
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		if (!strncmp(line,"root=",5)) {
// TODO WGJA WIP: 			int n;
// TODO WGJA WIP: 			line += 5;
// TODO WGJA WIP: 			if (strncmp(line,"/dev/",5)) {
// TODO WGJA WIP: 				ROOT_DEV = simple_strtoul(line,NULL,16);
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			line += 5;
// TODO WGJA WIP: 			for (n = 0 ; devnames[n] ; n++) {
// TODO WGJA WIP: 				int len = strlen(devnames[n]);
// TODO WGJA WIP: 				if (!strncmp(line,devnames[n],len)) {
// TODO WGJA WIP: 					ROOT_DEV = devnums[n]+simple_strtoul(line+len,NULL,16);
// TODO WGJA WIP: 					break;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		} else if (!strcmp(line,"ro"))
// TODO WGJA WIP: 			root_mountflags |= MS_RDONLY;
// TODO WGJA WIP: 		else if (!strcmp(line,"rw"))
// TODO WGJA WIP: 			root_mountflags &= ~MS_RDONLY;
// TODO WGJA WIP: 		else if (!strcmp(line,"no387")) {
// TODO WGJA WIP: 			hard_math = 0;
// TODO WGJA WIP: 			__asm__("movl %%cr0,%%eax\n\t"
// TODO WGJA WIP: 				"orl $0xE,%%eax\n\t"
// TODO WGJA WIP: 				"movl %%eax,%%cr0\n\t" : : : "ax");
// TODO WGJA WIP: 		} else
// TODO WGJA WIP: 			checksetup(line);
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * Then check if it's an environment variable or
// TODO WGJA WIP: 		 * an option.
// TODO WGJA WIP: 		 */	
// TODO WGJA WIP: 		if (strchr(line,'=')) {
// TODO WGJA WIP: 			if (envs >= MAX_INIT_ENVS)
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 			envp_init[++envs] = line;
// TODO WGJA WIP: 		} else {
// TODO WGJA WIP: 			if (args >= MAX_INIT_ARGS)
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 			argv_init[++args] = line;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	argv_init[args+1] = NULL;
// TODO WGJA WIP: 	envp_init[envs+1] = NULL;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static void copro_timeout(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	fpu_error = 1;
// TODO WGJA WIP: 	timer_table[COPRO_TIMER].expires = jiffies+100;
// TODO WGJA WIP: 	timer_active |= 1<<COPRO_TIMER;
// TODO WGJA WIP: 	printk("387 failed: trying to reset\n");
// TODO WGJA WIP: 	send_sig(SIGFPE, last_task_used_math, 1);
// TODO WGJA WIP: 	outb_p(0,0xf1);
// TODO WGJA WIP: 	outb_p(0,0xf0);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
extern "C" void start_kernel(void)
{
	// For easy work in progress early kernel debugging
	init_early_printk();
/*
 * Interrupts are still disabled. Do necessary setups, then
 * enable them
 */
	set_call_gate(&default_ldt,lcall7);
// TODO WGJA WIP:  	ROOT_DEV = ORIG_ROOT_DEV;
// TODO WGJA WIP:  	drive_info = DRIVE_INFO;
// TODO WGJA WIP:  	screen_info = SCREEN_INFO;
// TODO WGJA WIP: 	aux_device_present = AUX_DEVICE_INFO;
// TODO WGJA WIP: 	memory_end = (1<<20) + (EXT_MEM_K<<10);
// TODO WGJA WIP: 	memory_end &= PAGE_MASK;
// TODO WGJA WIP: 	ramdisk_size = RAMDISK_SIZE;
// TODO WGJA WIP: 	strcpy(command_line,COMMAND_LINE);
// TODO WGJA WIP: #ifdef CONFIG_MAX_16M
// TODO WGJA WIP: 	if (memory_end > 16*1024*1024)
// TODO WGJA WIP: 		memory_end = 16*1024*1024;
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	if (MOUNT_ROOT_RDONLY)
// TODO WGJA WIP: 		root_mountflags |= MS_RDONLY;
// TODO WGJA WIP: 	if ((unsigned long)&end >= (1024*1024)) {
// TODO WGJA WIP: 		memory_start = (unsigned long) &end;
// TODO WGJA WIP: 		low_memory_start = PAGE_SIZE;
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		memory_start = 1024*1024;
// TODO WGJA WIP: 		low_memory_start = (unsigned long) &end;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	low_memory_start = PAGE_ALIGN(low_memory_start);
// TODO WGJA WIP: 	memory_start = paging_init(memory_start,memory_end);
// TODO WGJA WIP: 	trap_init();
// TODO WGJA WIP: 	init_IRQ();
// TODO WGJA WIP: 	sched_init();
// TODO WGJA WIP: 	parse_options(command_line);
// TODO WGJA WIP: #ifdef CONFIG_PROFILE
// TODO WGJA WIP: 	prof_buffer = (unsigned long *) memory_start;
// TODO WGJA WIP: 	prof_len = (unsigned long) &end;
// TODO WGJA WIP: 	prof_len >>= 2;
// TODO WGJA WIP: 	memory_start += prof_len * sizeof(unsigned long);
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	memory_start = chr_dev_init(memory_start,memory_end);
// TODO WGJA WIP: 	memory_start = blk_dev_init(memory_start,memory_end);
// TODO WGJA WIP: #ifdef CONFIG_SCSI
// TODO WGJA WIP: 	memory_start = scsi_dev_init(memory_start,memory_end);
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	memory_start = inode_init(memory_start,memory_end);
// TODO WGJA WIP: 	memory_start = file_table_init(memory_start,memory_end);
// TODO WGJA WIP: 	mem_init(low_memory_start,memory_start,memory_end);
// TODO WGJA WIP: 	buffer_init();
// TODO WGJA WIP: 	time_init();
// TODO WGJA WIP: 	floppy_init();
// TODO WGJA WIP: 	sock_init();
// TODO WGJA WIP: #ifdef CONFIG_SYSVIPC
// TODO WGJA WIP: 	ipc_init();
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	sti();
// TODO WGJA WIP: 	calibrate_delay();
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * check if exception 16 works correctly.. This is truly evil
// TODO WGJA WIP: 	 * code: it disables the high 8 interrupts to make sure that
// TODO WGJA WIP: 	 * the irq13 doesn't happen. But as this will lead to a lockup
// TODO WGJA WIP: 	 * if no exception16 arrives, it depends on the fact that the
// TODO WGJA WIP: 	 * high 8 interrupts will be re-enabled by the next timer tick.
// TODO WGJA WIP: 	 * So the irq13 will happen eventually, but the exception 16
// TODO WGJA WIP: 	 * should get there first..
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if (hard_math) {
// TODO WGJA WIP: 		unsigned short control_word;
// TODO WGJA WIP: 
// TODO WGJA WIP: 		printk("Checking 386/387 coupling... ");
// TODO WGJA WIP: 		timer_table[COPRO_TIMER].expires = jiffies+50;
// TODO WGJA WIP: 		timer_table[COPRO_TIMER].fn = copro_timeout;
// TODO WGJA WIP: 		timer_active |= 1<<COPRO_TIMER;
// TODO WGJA WIP: 		__asm__("clts ; fninit ; fnstcw %0 ; fwait":"=m" (*&control_word));
// TODO WGJA WIP: 		control_word &= 0xffc0;
// TODO WGJA WIP: 		__asm__("fldcw %0 ; fwait": :"m" (*&control_word));
// TODO WGJA WIP: 		outb_p(inb_p(0x21) | (1 << 2), 0x21);
// TODO WGJA WIP: 		__asm__("fldz ; fld1 ; fdiv %st,%st(1) ; fwait");
// TODO WGJA WIP: 		timer_active &= ~(1<<COPRO_TIMER);
// TODO WGJA WIP: 		if (!fpu_error)
// TODO WGJA WIP: 			printk("Ok, fpu using %s error reporting.\n",
// TODO WGJA WIP: 				ignore_irq13?"exception 16":"irq13");
// TODO WGJA WIP: 	}
// TODO WGJA WIP: #ifndef CONFIG_MATH_EMULATION
// TODO WGJA WIP: 	else {
// TODO WGJA WIP: 		printk("No coprocessor found and no math emulation present.\n");
// TODO WGJA WIP: 		printk("Giving up.\n");
// TODO WGJA WIP: 		for (;;) ;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	move_to_user_mode();
// TODO WGJA WIP: 	if (!fork())		/* we count on this going ok */
// TODO WGJA WIP: 		init();
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * task[0] is meant to be used as an "idle" task: it may not sleep, but
// TODO WGJA WIP:  * it might do some general things like count free pages or it could be
// TODO WGJA WIP:  * used to implement a reasonable LRU algorithm for the paging routines:
// TODO WGJA WIP:  * anything that can be useful, but shouldn't take time from the real
// TODO WGJA WIP:  * processes.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Right now task[0] just does a infinite idle loop.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 	for(;;)
// TODO WGJA WIP: 		idle();
}
// TODO WGJA WIP: 
// TODO WGJA WIP: static int printf(const char *fmt, ...)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	va_list args;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	va_start(args, fmt);
// TODO WGJA WIP: 	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
// TODO WGJA WIP: 	va_end(args);
// TODO WGJA WIP: 	return i;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void init(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int pid,i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	setup((void *) &drive_info);
// TODO WGJA WIP: 	(void) open("/dev/tty1",O_RDWR,0);
// TODO WGJA WIP: 	(void) dup(0);
// TODO WGJA WIP: 	(void) dup(0);
// TODO WGJA WIP: 
// TODO WGJA WIP: 	system_utsname.machine[1] = '0' + x86;
// TODO WGJA WIP: 	printf(linux_banner);
// TODO WGJA WIP: 
// TODO WGJA WIP: 	execve("/etc/init",argv_init,envp_init);
// TODO WGJA WIP: 	execve("/bin/init",argv_init,envp_init);
// TODO WGJA WIP: 	execve("/sbin/init",argv_init,envp_init);
// TODO WGJA WIP: 	/* if this fails, fall through to original stuff */
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!(pid=fork())) {
// TODO WGJA WIP: 		close(0);
// TODO WGJA WIP: 		if (open("/etc/rc",O_RDONLY,0))
// TODO WGJA WIP: 			exit(1);
// TODO WGJA WIP: 		execve("/bin/sh",argv_rc,envp_rc);
// TODO WGJA WIP: 		exit(2);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (pid>0)
// TODO WGJA WIP: 		while (pid != wait(&i))
// TODO WGJA WIP: 			/* nothing */;
// TODO WGJA WIP: 	while (1) {
// TODO WGJA WIP: 		if ((pid = fork()) < 0) {
// TODO WGJA WIP: 			printf("Fork failed in init\n\r");
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!pid) {
// TODO WGJA WIP: 			close(0);close(1);close(2);
// TODO WGJA WIP: 			setsid();
// TODO WGJA WIP: 			(void) open("/dev/tty1",O_RDWR,0);
// TODO WGJA WIP: 			(void) dup(0);
// TODO WGJA WIP: 			(void) dup(0);
// TODO WGJA WIP: 			exit(execve("/bin/sh",argv,envp));
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		while (1)
// TODO WGJA WIP: 			if (pid == wait(&i))
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 		printf("\n\rchild %d died with code %04x\n\r",pid,i);
// TODO WGJA WIP: 		sync();
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	exit(0);
// TODO WGJA WIP: }
