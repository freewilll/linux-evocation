/*
 *  linux/fs/fcntl.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/string.h>

extern int fcntl_getlk(unsigned int, struct flock *);
extern int fcntl_setlk(unsigned int, unsigned int, struct flock *);
extern int sock_fcntl (struct file *, unsigned int cmd, unsigned long arg);

static int dupfd(unsigned int fd, unsigned int arg)
{
	if (fd >= NR_OPEN || !current->filp[fd])
		return -EBADF;
	if (arg >= NR_OPEN)
		return -EINVAL;
	while (arg < NR_OPEN)
		if (current->filp[arg])
			arg++;
		else
			break;
	if (arg >= NR_OPEN)
		return -EMFILE;
	FD_CLR(arg, &current->close_on_exec);
	(current->filp[arg] = current->filp[fd])->f_count++;
	return arg;
}

// TODO WGJA WIP: extern "C" int sys_dup2(unsigned int oldfd, unsigned int newfd)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (oldfd >= NR_OPEN || !current->filp[oldfd])
// TODO WGJA WIP: 		return -EBADF;
// TODO WGJA WIP: 	if (newfd == oldfd)
// TODO WGJA WIP: 		return newfd;
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * errno's for dup2() are slightly different than for fcntl(F_DUPFD)
// TODO WGJA WIP: 	 * for historical reasons.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if (newfd > NR_OPEN)	/* historical botch - should have been >= */
// TODO WGJA WIP: 		return -EBADF;	/* dupfd() would return -EINVAL */
// TODO WGJA WIP: #if 1
// TODO WGJA WIP: 	if (newfd == NR_OPEN)
// TODO WGJA WIP: 		return -EBADF;	/* dupfd() does return -EINVAL and that may
// TODO WGJA WIP: 				 * even be the standard!  But that is too
// TODO WGJA WIP: 				 * weird for now.
// TODO WGJA WIP: 				 */
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 	sys_close(newfd);
// TODO WGJA WIP: 	return dupfd(oldfd,newfd);
// TODO WGJA WIP: }

extern "C" int sys_dup(unsigned int fildes)
{
	return dupfd(fildes,0);
}

// TODO WGJA WIP: extern "C" int sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
// TODO WGJA WIP: {	
// TODO WGJA WIP: 	struct file * filp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (fd >= NR_OPEN || !(filp = current->filp[fd]))
// TODO WGJA WIP: 		return -EBADF;
// TODO WGJA WIP: 	switch (cmd) {
// TODO WGJA WIP: 		case F_DUPFD:
// TODO WGJA WIP: 			return dupfd(fd,arg);
// TODO WGJA WIP: 		case F_GETFD:
// TODO WGJA WIP: 			return FD_ISSET(fd, &current->close_on_exec);
// TODO WGJA WIP: 		case F_SETFD:
// TODO WGJA WIP: 			if (arg&1)
// TODO WGJA WIP: 				FD_SET(fd, &current->close_on_exec);
// TODO WGJA WIP: 			else
// TODO WGJA WIP: 				FD_CLR(fd, &current->close_on_exec);
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		case F_GETFL:
// TODO WGJA WIP: 			return filp->f_flags;
// TODO WGJA WIP: 		case F_SETFL:
// TODO WGJA WIP: 			filp->f_flags &= ~(O_APPEND | O_NONBLOCK);
// TODO WGJA WIP: 			filp->f_flags |= arg & (O_APPEND | O_NONBLOCK);
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		case F_GETLK:
// TODO WGJA WIP: 			return fcntl_getlk(fd, (struct flock *) arg);
// TODO WGJA WIP: 		case F_SETLK:
// TODO WGJA WIP: 			return fcntl_setlk(fd, cmd, (struct flock *) arg);
// TODO WGJA WIP: 		case F_SETLKW:
// TODO WGJA WIP: 			return fcntl_setlk(fd, cmd, (struct flock *) arg);
// TODO WGJA WIP: 		default:
// TODO WGJA WIP: 			/* sockets need a few special fcntls. */
// TODO WGJA WIP: 			if (S_ISSOCK (filp->f_inode->i_mode))
// TODO WGJA WIP: 			  {
// TODO WGJA WIP: 			     return (sock_fcntl (filp, cmd, arg));
// TODO WGJA WIP: 			  }
// TODO WGJA WIP: 			return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
