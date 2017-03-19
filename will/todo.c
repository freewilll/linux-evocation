#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/tty.h>
#include <linux/user.h>

// TODO WGJA syscall_trace 
extern "C" int syscall_trace(void)
{
	printk("TODO: syscall_trace\n");
	return -ENOSYS;
}

// TODO WGJA sys_ptrace 
extern "C" int sys_ptrace(void)
{
	printk("TODO: sys_ptrace\n");
	return -ENOSYS;
}

// TODO WGJA sys_alarm 
extern "C" int sys_alarm(void)
{
	printk("TODO: sys_alarm\n");
	return -ENOSYS;
}

// TODO WGJA sys_fcntl 
extern "C" int sys_fcntl(void)
{
	printk("TODO: sys_fcntl\n");
	return -ENOSYS;
}

// TODO WGJA sys_select 
extern "C" int sys_select(void)
{
	printk("TODO: sys_select\n");
	return -ENOSYS;
}

// TODO WGJA sys_uselib 
extern "C" int sys_uselib(void)
{
	printk("TODO: sys_uselib\n");
	return -ENOSYS;
}

// TODO WGJA sys_swapon 
extern "C" int sys_swapon(void)
{
	printk("TODO: sys_swapon\n");
	return -ENOSYS;
}

// TODO WGJA sys_socketcall 
extern "C" int sys_socketcall(void)
{
	printk("TODO: sys_socketcall\n");
	return -ENOSYS;
}

// TODO WGJA sys_setitimer 
extern "C" int sys_setitimer(void)
{
	printk("TODO: sys_setitimer\n");
	return -ENOSYS;
}

// TODO WGJA sys_getitimer 
extern "C" int sys_getitimer(void)
{
	printk("TODO: sys_getitimer\n");
	return -ENOSYS;
}

// TODO WGJA sys_swapoff 
extern "C" int sys_swapoff(void)
{
	printk("TODO: sys_swapoff\n");
	return -ENOSYS;
}

// TODO WGJA sys_sysinfo 
extern "C" int sys_sysinfo(void)
{
	printk("TODO: sys_sysinfo\n");
	return -ENOSYS;
}

// TODO WGJA sys_ipc 
extern "C" int sys_ipc(void)
{
	printk("TODO: sys_ipc\n");
	return -ENOSYS;
}

// TODO WGJA sys_modify_ldt 
extern "C" int sys_modify_ldt(void)
{
	printk("TODO: sys_modify_ldt\n");
	return -ENOSYS;
}


// kernel/kernel.o:(.data+0xb48): undefined reference to `sys_ptrace'
// kernel/kernel.o:(.data+0xb4c): undefined reference to `sys_alarm'
// kernel/kernel.o:(.data+0xbbc): undefined reference to `sys_fcntl'
// kernel/kernel.o:(.data+0xc28): undefined reference to `sys_select'
// kernel/kernel.o:(.data+0xc38): undefined reference to `sys_uselib'
// kernel/kernel.o:(.data+0xc3c): undefined reference to `sys_swapon'
// kernel/kernel.o:(.data+0xc78): undefined reference to `sys_socketcall'
// kernel/kernel.o:(.data+0xc80): undefined reference to `sys_setitimer'
// kernel/kernel.o:(.data+0xc84): undefined reference to `sys_getitimer'
// kernel/kernel.o:(.data+0xcac): undefined reference to `sys_swapoff'
// kernel/kernel.o:(.data+0xcb0): undefined reference to `sys_sysinfo'
// kernel/kernel.o:(.data+0xcb4): undefined reference to `sys_ipc'
// kernel/kernel.o:(.data+0xccc): undefined reference to `sys_modify_ldt'

// // TODO WGJA syscall_trace
// extern "C" void syscall_trace(void)
// {
// 	printk("TODO: syscall_trace\n");
// 	for (;;);
// }

// // TODO WGJA syscalls
// extern "C" int sys_todo(struct pt_regs regs)
// {
// 	printk("todo syscall %#x (%d).\n", regs.orig_eax, regs.orig_eax);
// 	return -ENOSYS;
// }

// TODO WGJA move sound_mem_init to real thing
void sound_mem_init(void)
{
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
