#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/unistd.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <linux/fcntl.h>
#include <linux/tty.h>

static inline _syscall0(int,fork)
static inline _syscall3(int,reboot,int,magic,int,magic_too,int,flag)
static inline _syscall0(int,idle)
static inline _syscall3(int,open,const char *,file,int,flag,int,mode)
static inline _syscall3(int,read,unsigned int,fd,char *,buf,unsigned int,count)
static inline _syscall3(int,readdir,unsigned int,fd,struct dirent *,dirent,unsigned int,count)
static inline _syscall2(int,mkdir,const char *,pathname,int,mode)
static inline _syscall1(int,rmdir,const char*,pathname)

extern unsigned long timer_active;

// TODO WGJA log_to_console & real prink
int log_to_console = 1;

// TODO WGJA do_signal
extern "C" int do_signal(unsigned long oldmask, struct pt_regs * regs)
{
	printk("TODO: do_signal\n");
	for (;;);
}

// TODO WGJA syscall_trace
extern "C" void syscall_trace(void)
{
	printk("TODO: syscall_trace\n");
	for (;;);
}

// TODO WGJA save_v86_state
extern "C" unsigned long save_v86_state(struct vm86_regs * regs)
{
	printk("TODO: save_v86_state\n");
	for (;;);
}

// TODO WGJA send_sig
int send_sig(unsigned long sig,struct task_struct * p,int priv)
{
	return 0;
}

// TODO WGJA syscalls
extern "C" int sys_todo(void)
{
	printk("todo syscall.\n");
	return -ENOSYS;
}

// TODO WGJA move sound_mem_init to real thing
void sound_mem_init(void)
{
}

// TODO WGJA shm_swap
int shm_swap(int prio)
{
	printk("TODO shm_swap\n");
	return 0;
}

static inline void * __memset_generic(void * s, char c,size_t count)
{
int d0, d1;
__asm__ __volatile__(
	"rep\n\t"
	"stosb"
	: "=&c" (d0), "=&D" (d1)
	:"a" (c),"1" (s),"0" (count)
	:"memory");
return s;
}


void test_fork() {
	int pid;
	char *vidmem = (char *)0xb8000;
	int i;

	for (i=0; i < 10; i++) {
		pid = fork();
		if (pid < 0) {
			vidmem[0] = '!';
			for (;;);
		}
		if (pid == 0) {
			printk("Forked pid=%d\n", i);
			for (;;) {
				printk("%d", i);
				idle();
			}
		}
	}
}

void __attribute__ ((noinline)) do_nothing(char* s, ...) {
	char *vidmem = (char *)0xb8000;
	vidmem[79*2] = 'A';
}

void test_fork_memory() {
	unsigned int i = 0;
	int pid;
	char *vidmem = (char *)0xb8000;

	printk("need_resched=%d\n", need_resched);
	need_resched = 0;  // Force task 0 to run first in qemu slow case

	if (pid = fork()) {
		i++;
		vidmem[0] = i + 48;
		vidmem[2] = pid + 48;
	}
	else {
		vidmem[160 + 0] = i + 48;
		vidmem[160 + 2] = pid + 48;
	}

	printk("pid=%d i=%d\n", pid, i);
	do_nothing("", i, pid);

	vidmem[320 + pid * 2] = 48 + i;
	do_nothing("", i, &i);
	vidmem[480 + pid * 2] = 48 + i;

	int esp, ebp;
	asm("\t movl %%esp,%0" : "=r"(esp));
	printk("pid=%d, esp=%#x\n", pid, esp);

	for(;;) idle();
}

void test_page_map() 
{
	int i=0;
	for (i=0;i<12;i++) {
		printk("\nFree pages %d\n", nr_free_pages);
		unsigned long page = __get_free_page(GFP_KERNEL);
		printk("Got page %x\n", page);
		free_page(page);
		printk("Free pages %d\n", nr_free_pages);
	}
}

void test_kmalloc()
{
	int i=0;

	// This statement will cause the same page to be reused, and it
	// won't get alloc'd and freed all the time.
	void* first = kmalloc(32, GFP_KERNEL);

	for (i=0;i<12;i++) {
		void* foo = kmalloc(32, GFP_KERNEL);
		printk("Free pages %d, foo=%x\n", nr_free_pages, foo);
		// kfree(foo);
	}
}

// TODO WGJA do_page_fault
extern "C" void do_page_fault(struct pt_regs *regs, unsigned long error_code)
{
	unsigned long address;
	unsigned long user_esp = 0;
	unsigned long stack_limit;
	unsigned int bit;

	char *vidmem = (char *)0xb8000;
	vidmem[79 * 2] = 'P';
	vidmem[79 * 2 + 1] = 0x40;

	/* get the address */
	__asm__("movl %%cr2,%0":"=r" (address));
	printk("Unable to handle kernel paging request at address %08x\n",address);

	for (;;);
}

void test_memset() {
	char* foo = "testing fkjlfdas jklfdsajklf dasjklfdaskjldfsa kjl fadsjklfd sakljfdsa kljfd sajklf ads";
	int c;
	c = 20;
	memset((void*) foo, 32, c);
	printk("%d\n", c);
	printk("%s\n", foo);
	// printk("%#x\n", &foo);
}

void test_memcpy() {
	char* to = "testing fkjlfdas jklfdsajklf dasjklfdaskjldfsa kjl fadsjklfd sakljfdsa kljfd sajklf ads";
	char* from = "xxxxxxxxxxxxxxxxxxxx";
	memcpy(to, from, 8);
	printk("%s\n", to);
}

// WGJA Add a temporary keyboard handler to be able to check aliveness
#define KEYBOARD_IRQ 1
static void keyboard_interrupt(int int_pt_regs)
{
	// If we don't read the scan code, they keyboard controller won't send another one
	unsigned char scancode;
	char *vidmem = (char *)0xb8000;

	if (!(inb_p(0x64) & 0x01)) {
		printk("Keyboard interrupt - bad keyb read.\n");
		goto end_kbd_intr;
	}
	scancode = inb(0x60);
	printk("%c", scancode);

	vidmem[0] = scancode;

	end_kbd_intr:
	return;
}

void init_test_keyboard()
{
	// WGJA Add a temporary keyboard handler to be able to check aliveness
	request_irq(KEYBOARD_IRQ, keyboard_interrupt);

}

void test_dev_zero()
{
	int fd;
	char buffer[16];
	int c;

	fd = open("/dev/zero", O_RDWR, 0);
	if (fd < 0) printk("Could not open /dev/zero: %d\n", fd);

	printk("fd=%d\n", fd);
	c = read(fd, buffer, 10);
	printk("Read %d bytes\n", c);
	printk("First byte: %d\n", buffer[0]);

}

// TODO WGJA generic_mmap
int generic_mmap(struct inode * inode, struct file * file,
	unsigned long addr, size_t len, int prot, unsigned long off)
{
	printk("TODO generic_mmap\n");
	for (;;);

}

void test_minix_mkdir() {
	int fd;
	printk("\nmkdir\n");
	fd = mkdir("/dir1", 0777);
	printk("  fd=%d\n", fd);
	fd = mkdir("/dir2", 0777);
	printk("  fd=%d\n", fd);
}

void test_minix_readdir() 
{
	printk("\nreaddir\n");
	int fd, c, i;
	fd = open("/", O_RDONLY, 0);
	if (fd < 0) printk("  Bad open\n");

	dirent d[10];
	c = 1;
	while (c > 0) {
		c = readdir(fd, d, 10);
		if (c < 0) panic("Bad readdir");
		if (c)
			printk("   %d %s\n", d[i].d_ino, d[i].d_name);
	}
}

void test_minix_rmdir()
{
	printk("\nrmdir\n");
	int r;
	r = rmdir("/dir2");
	if (r < 0) printk("  Bad rmdir\n");
}

void test_minix() 
{
	test_minix_mkdir();
	test_minix_readdir();
	test_minix_rmdir();
	test_minix_readdir();
}

void test_tty1_read()
{
	int stdin, c;
	stdin = open("/dev/tty1",O_RDWR,0);
	printk("stdin=%d\n", stdin);

	// printk("Trying to read from tty1\n");
	char buf[10];
	for(;;) {
		c = read(stdin, buf, 10);
		if (c < 0) {
			printk("Got %d\n", c);
			panic("Bad shit: c < 0");
		}
		buf[c - 1] = 0; // Strip \n
		printk("Read: %02d bytes: '%s'\n", c, buf);
		
		if (!strcmp("reboot\n", buf)) {
			// To test hard_reset_now()
			reboot(0xfee1dead, 672274793, 0x01234567);
		}
	}
}

void do_keyboard_interrupt(void)
{
	// printk("do_keyboard_interrupt\n");
	TTY_READ_FLUSH(TTY_TABLE(0));
	timer_active &= ~(1<<0);
	// printk("do_keyboard_interrupt done\n");
	// TODO WGJA, rest of do_keyboard_interrupt
	return;
}

// TODO WGJA scrollback
void scrollback(int i)
{
	printk("TODO: scrollback\n");
}

// TODO WGJA scrollfront
void scrollfront(int i)
{
	printk("TODO: scrollfront\n");
}

// TODO WGJA change_console
void change_console(unsigned int)
{
	printk("TODO: change_console\n");
}

// TODO WGJA con_write
void con_write(struct tty_struct * tty)
{
	// printk("TODO WGJA con_write\n");
}

// TODO WGJA con_open
int con_open(struct tty_struct *tty, struct file * filp)
{
	printk("// TODO WGJA con_open\n");
	tty->write = con_write;
	return 0;
}

unsigned long video_num_lines;
unsigned long video_num_columns;


// TODO WGJA rs_open
int rs_open(struct tty_struct *tty, struct file * filp)
{
	printk("// TODO WGJA rs_open\n");
	tty->write = con_write;
	return 0;
}

// TODO WGJA pty_open
int pty_open(struct tty_struct *tty, struct file * filp)
{
	printk("// TODO WGJA pty_open\n");
	tty->write = con_write;
	return 0;
}

// TODO WGJA con_init
long con_init(long kmem_start)
{
	printk("TODO WGJA con_init\n");
	return kmem_start;
}

// TODO WGJA rs_init
long rs_init(long kmem_start)
{
	printk("TODO WGJA rs_init\n");
	return kmem_start;
}

// TODO WGJA do_screendump
int do_screendump(int arg)
{
	printk("TODO WGJA do_screendump\n");
}
