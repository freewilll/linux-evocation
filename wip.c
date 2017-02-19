#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/unistd.h>

static inline _syscall0(int,fork)
static inline _syscall0(int,idle)

// TODO WGJA panic
void panic(const char * s)
{
	printk("In swapper task - not syncing\n");
}

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

// TODO WGJA shrink_buffers
int shrink_buffers(unsigned int prio)
{
	printk("TODO shrink_buffers\n");
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

// TODO WGJA get_empty_filp
struct file * get_empty_filp(void)
{
	printk("TODO get_empty_filp\n");
	for (;;);
}

// TODO WGJA iput
void iput(struct inode * inode)
{
	printk("TODO iput\n");
	for (;;);
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