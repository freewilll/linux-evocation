#include <stdarg.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <linux/fcntl.h>
#include <linux/mman.h>
#include <asm/segment.h>

extern "C" int vsprintf(char *,const char *,va_list);

static inline _syscall0(int,fork)
static inline _syscall3(int,reboot,int,magic,int,magic_too,int,flag)
static inline _syscall0(int,idle)
static inline _syscall3(int,open,const char *,file,int,flag,int,mode)
static inline _syscall3(int,read,unsigned int,fd,char *,buf,unsigned int,count)
static inline _syscall3(int,write,int,fd,const char *,buf,off_t,count)
static inline _syscall1(int,close,unsigned int,fd)
static inline _syscall3(int,readdir,unsigned int,fd,struct dirent *,dirent,unsigned int,count)
static inline _syscall2(int,mkdir,const char *,pathname,int,mode)
static inline _syscall1(int,rmdir,const char*,pathname)
static inline _syscall1(int,dup,int,fd)
static inline _syscall1(int,mmap,unsigned long*,buffer)


extern inline void * memmove(void * dest,const void * src, size_t n);

static char printbuf[1024];

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

static int printf(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}

// http://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

// http://wiki.osdev.org/PS/2_Mouse
void test_dev_psaux()
{
	int fd;
	char buffer[16];
	int c;

	fd = open("/dev/ps2aux", O_RDONLY, 0);
	if (fd < 0) printf("Could not open /dev/ps2aux: %d\n", fd);

	printf("fd=%d\n", fd);
	while (1) {
		c = read(fd, buffer, 3);
		if (c < 0) 
			printf("Bad read\n");
		else if (c != 3) 
			printf("Didn't get 3 bytes\n");
		else {
			printf("Got " BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buffer[0]));
			printf(" %d %d\n", buffer[1], buffer[2]);
		}
	}

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
	(void) dup(0);
	(void) dup(0);

	printf("stdin=%d\n", stdin);

	// printk("Trying to read from tty1\n");
	char buf[10];
	for(;;) {
		c = read(stdin, buf, 10);
		if (c < 0) {
			printf("Got %d\n", c);
			panic("Bad shit: c < 0");
		}
		buf[c - 1] = 0; // Strip \n
		printf("Read: %02d bytes: '%s'\n", c, buf);
		
		if (!strcmp("reboot\n", buf)) {
			// To test hard_reset_now()
			reboot(0xfee1dead, 672274793, 0x01234567);
		}
	}
}

void test_printf() 
{
	int i;

	// Reverse linefeed (RI) test
	// for(i=0;i<50;i++)
	// 	printf("\eM");  // Esc-M  = RI       Reverse linefeed
	// 	// printf("x");

	// print_vc_cons_mem_situation();

	// Line feed test
	// The 180 isn't coincidental. It tests both code paths of the video memory
	// filling up and being full in scrup(). Line 176 triggers full memory.
	// for(i=0;i<180;i++)
	// 	printf("%d\n", i);

	// Clear screen
	// printf("\e[2J");

	// ESC-K test
	// printf("Hello\n\e[1A"); // Print hello & move cursor up
	// printf("\e[0K");  // Clear from cursor to EOL
	// for(;;) idle();

	// // Color test
	// printf("\e[%dmBlack\n",  30 + 0);  // Not really black?
	// printf("\e[%dmRed\n", 	 30 + 1);  
	// printf("\e[%dmGreen\n",  30 + 2);
	// printf("\e[%dmYellow\n", 30 + 3);
	// printf("\e[%dmBlue\n",   30 + 4);
	// printf("\e[%dmMagenta\n",30 + 5);
	// printf("\e[%dmCyan\n",   30 + 6);
	// printf("\e[%dmWhite\n",  30 + 7);
	// printf("\e[0m"); // Reset
}

void test_memmove() 
{
	char* buf1 = "foo\0";
	char* buf2 = "bar\0";

	printf("\n");
	printf("1 %s\n", buf1);  
	printf("1 %s\n", buf2);

	// Move buf1 -> buf2
	memmove(buf2, buf1, 3);  // dest, src, size
	printf("\n");
	printf("2 %s\n", buf1);  
	printf("2 %s\n", buf2);
	if (memcmp(buf1, buf2, 3) != 0)
		panic("Buffers are the same (1)");
	if (memcmp(buf1, "foo", 3) != 0)
		panic("Bad buffer contents (1)");

	// Note: tempting as it may be to call this "bar\0", that 
	// leads to incorrect code, since gcc optimizes things so that
	// both "bar\0" strings point at the same memory address. This will
	// lead to buf2 being "foo\0", even with a buf2 = "bar\n" statement.
	buf2 = "baz\0";

	printf("\n");
	printf("3 %s\n", buf1);  
	printf("3 %s\n", buf2);

	// Move buf2 -> buf1
	memmove(buf1, buf2, 3);  // dest, src, size
	printf("\n");
	printf("4 %s\n", buf1);  
	printf("4 %s\n", buf2);
	if (memcmp(buf1, buf2, 3) != 0)
		panic("Buffers are the same (2)");
	if (memcmp(buf1, "baz", 3) != 0)
		panic("Bad buffer contents (2)");
}

void test_general_protection_fault()
{
	int pid;
	pid = fork();
	if (pid < 0) panic("Can't fork");
	if (!pid) {
		sti();  // Goodbye cruel world
		printf("No way should we get here");
	}
}

void test_file_read_write()
{
	int fd, res, c;
	fd = open("/foo", O_RDWR | O_CREAT, 0);
	if (fd < 0) panic("Bad fd");

	char* buf = "This is the time for all good men to ...\0";
	write(fd, buf, strlen(buf));
	res = close(fd);
	if (res < 0) panic("Bad close");
	printf("File created ok\n");

	fd = open("/foo", O_RDONLY, 0);
	if (fd < 0) panic("Bad fd");

	char buf2[20];
	c = read(fd, buf2, 100);
	printf("Read %d\n", c);
	if (c < 0) panic("Bad read");
	printf("buf: %s\n", buf);
	printf("buf2: %s\n", buf2);
	res = close(fd);
	if (res < 0) panic("Bad close");
}

void test_mmap()
{
	int fd, res, c;
	fd = open("/foo", O_RDWR | O_CREAT, 0);
	if (fd < 0) panic("Bad fd");

	char* buf = "This is the time for all good men to ...\0";
	write(fd, buf, strlen(buf));
	res = close(fd);
	if (res < 0) panic("Bad close");
	printf("File created ok\n");

	fd = open("/foo", O_RDONLY, 0);
	if (fd < 0) panic("Bad fd");

	unsigned long parms[6] = {
		NULL,		// void *addr
		0x10,		// size_t length,
		PROT_READ,	// int prot,
		MAP_SHARED,	// int flags,
		fd,		// int fd,
		0x0		// off_t offset
	};

	char* mapped;
	mapped = (char*) mmap(parms);
	printf("mmap result: %#x\n", mapped);
	if ((int) mapped < 0) panic("Bad mmap");

	// Let's poke into user space and see what's there then ...
	char buf2[100];
	memcpy_fromfs(buf2, mapped, 0x10);
	buf2[0x10] = 0;
	printf("mmapped read=%s\n", buf2);
}

void test_strtok()
{
	// https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
	char str[80] = "This is - www.tutorialspoint.com - website";
	const char s[2] = "-";
	char *token;

	/* get the first token */
	token = strtok(str, s);

	/* walk through other tokens */
	while( token != NULL ) 
	{
		printf( " %s\n", token );
		token = strtok(NULL, s);
	}
}

// http://www.cplusplus.com/reference/cstring/strrchr/
void test_strrchr()
{
	char str[] = "This is a sample string";
	char * pch;
	pch=strrchr(str,'s');
	printf ("Last occurence of 's' found at %d \n",pch-str+1);
}

// http://www.cplusplus.com/reference/cstring/strcat/
void test_strcat()
{
	char str[80];
	strcpy (str,"these ");
	strcat (str,"strings ");
	strcat (str,"are ");
	strcat (str,"concatenated.");
	printf("%s\n", str);
}

// http://www.cplusplus.com/reference/cstring/strncat/
void test_strncat()
{
	char str1[20];
	char str2[20];
	strcpy (str1,"To be ");
	strcpy (str2,"or not to be");
	strncat (str1, str2, 6);
	printf("%s\n ", str1); 
}
