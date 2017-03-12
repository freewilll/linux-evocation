#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/tty.h>

// TODO WGJA syscall_trace
extern "C" void syscall_trace(void)
{
	printk("TODO: syscall_trace\n");
	for (;;);
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

// TODO WGJA generic_mmap
int generic_mmap(struct inode * inode, struct file * file,
	unsigned long addr, size_t len, int prot, unsigned long off)
{
	printk("TODO generic_mmap\n");
	for (;;);

}

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

// TODO WGJA rs_init
long rs_init(long kmem_start)
{
	printk("TODO WGJA rs_init\n");
	return kmem_start;
}

// TODO WGJA math_emulate
extern "C" void math_emulate(long arg)
{
	printk("TODO WGJA math_emulate\n");
}

// TODO WGJA core_dump
int core_dump(long signr, struct pt_regs * regs)
{
	printk("TODO WGJA core_dump\n");
}
