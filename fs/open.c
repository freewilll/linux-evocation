/*
 *  linux/fs/open.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/vfs.h>
#include <linux/types.h>
// TODO WGJA WIP: #include <linux/utime.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/tty.h>
#include <linux/time.h>

#include <asm/segment.h>

extern void fcntl_remove_locks(struct task_struct *, struct file *);

// TODO WGJA WIP: extern "C" int sys_ustat(int dev, struct ustat * ubuf)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return -ENOSYS;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_statfs(const char * path, struct statfs * buf)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE, buf, sizeof(struct statfs));
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	error = namei(path,&inode);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (!inode->i_sb->s_op->statfs) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -ENOSYS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	inode->i_sb->s_op->statfs(inode->i_sb, buf);
// TODO WGJA WIP: 	iput(inode);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_fstatfs(unsigned int fd, struct statfs * buf)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	struct file * file;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE, buf, sizeof(struct statfs));
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (fd >= NR_OPEN || !(file = current->filp[fd]))
// TODO WGJA WIP: 		return -EBADF;
// TODO WGJA WIP: 	if (!(inode = file->f_inode))
// TODO WGJA WIP: 		return -ENOENT;
// TODO WGJA WIP: 	if (!inode->i_sb->s_op->statfs)
// TODO WGJA WIP: 		return -ENOSYS;
// TODO WGJA WIP: 	inode->i_sb->s_op->statfs(inode->i_sb, buf);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_truncate(const char * path, unsigned int length)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = namei(path,&inode);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (S_ISDIR(inode->i_mode) || !permission(inode,MAY_WRITE)) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_RDONLY(inode)) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	inode->i_size = length;
// TODO WGJA WIP: 	if (inode->i_op && inode->i_op->truncate)
// TODO WGJA WIP: 		inode->i_op->truncate(inode);
// TODO WGJA WIP: 	inode->i_atime = inode->i_mtime = CURRENT_TIME;
// TODO WGJA WIP: 	inode->i_dirt = 1;
// TODO WGJA WIP: 	error = notify_change(NOTIFY_SIZE, inode);
// TODO WGJA WIP: 	iput(inode);
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_ftruncate(unsigned int fd, unsigned int length)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	struct file * file;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (fd >= NR_OPEN || !(file = current->filp[fd]))
// TODO WGJA WIP: 		return -EBADF;
// TODO WGJA WIP: 	if (!(inode = file->f_inode))
// TODO WGJA WIP: 		return -ENOENT;
// TODO WGJA WIP: 	if (S_ISDIR(inode->i_mode) || !(file->f_mode & 2))
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	inode->i_size = length;
// TODO WGJA WIP: 	if (inode->i_op && inode->i_op->truncate)
// TODO WGJA WIP: 		inode->i_op->truncate(inode);
// TODO WGJA WIP: 	inode->i_atime = inode->i_mtime = CURRENT_TIME;
// TODO WGJA WIP: 	inode->i_dirt = 1;
// TODO WGJA WIP: 	return notify_change(NOTIFY_SIZE, inode);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /* If times==NULL, set access and modification to current time,
// TODO WGJA WIP:  * must be owner or have write permission.
// TODO WGJA WIP:  * Else, update from *times, must be owner or super user.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_utime(char * filename, struct utimbuf * times)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	long actime,modtime;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = namei(filename,&inode);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (IS_RDONLY(inode)) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (times) {
// TODO WGJA WIP: 		if ((current->euid != inode->i_uid) && !suser()) {
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			return -EPERM;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		actime = get_fs_long((unsigned long *) &times->actime);
// TODO WGJA WIP: 		modtime = get_fs_long((unsigned long *) &times->modtime);
// TODO WGJA WIP: 		inode->i_ctime = CURRENT_TIME;
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		if ((current->euid != inode->i_uid) &&
// TODO WGJA WIP: 		    !permission(inode,MAY_WRITE)) {
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			return -EACCES;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		actime = modtime = inode->i_ctime = CURRENT_TIME;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	inode->i_atime = actime;
// TODO WGJA WIP: 	inode->i_mtime = modtime;
// TODO WGJA WIP: 	inode->i_dirt = 1;
// TODO WGJA WIP: 	error = notify_change(NOTIFY_TIME, inode);
// TODO WGJA WIP: 	iput(inode);
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * XXX we should use the real ids for checking _all_ components of the
// TODO WGJA WIP:  * path.  Now we only use them for the final component of the path.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_access(const char * filename,int mode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int res, i_mode;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (mode != (mode & S_IRWXO))	/* where's F_OK, X_OK, W_OK, R_OK? */
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	res = namei(filename,&inode);
// TODO WGJA WIP: 	if (res)
// TODO WGJA WIP: 		return res;
// TODO WGJA WIP: 	i_mode = inode->i_mode;
// TODO WGJA WIP: 	res = i_mode & S_IRWXUGO;
// TODO WGJA WIP: 	if (current->uid == inode->i_uid)
// TODO WGJA WIP: 		res >>= 6;		/* needs cleaning? */
// TODO WGJA WIP: 	else if (in_group_p(inode->i_gid))
// TODO WGJA WIP: 		res >>= 3;		/* needs cleaning? */
// TODO WGJA WIP: 	iput(inode);
// TODO WGJA WIP: 	if ((res & mode) == mode)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * XXX we are doing this test last because we really should be
// TODO WGJA WIP: 	 * swapping the effective with the real user id (temporarily),
// TODO WGJA WIP: 	 * and then calling suser() routine.  If we do call the
// TODO WGJA WIP: 	 * suser() routine, it needs to be called last. 
// TODO WGJA WIP: 	 *
// TODO WGJA WIP: 	 * XXX nope.  suser() is inappropriate and swapping the ids while
// TODO WGJA WIP: 	 * decomposing the path would be racy.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if ((!current->uid) &&
// TODO WGJA WIP: 	    (S_ISDIR(i_mode) || !(mode & S_IXOTH) || (i_mode & S_IXUGO)))
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	return -EACCES;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_chdir(const char * filename)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = namei(filename,&inode);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (!S_ISDIR(inode->i_mode)) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -ENOTDIR;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!permission(inode,MAY_EXEC)) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	iput(current->pwd);
// TODO WGJA WIP: 	current->pwd = inode;
// TODO WGJA WIP: 	return (0);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_chroot(const char * filename)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = namei(filename,&inode);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (!S_ISDIR(inode->i_mode)) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -ENOTDIR;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!suser()) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	iput(current->root);
// TODO WGJA WIP: 	current->root = inode;
// TODO WGJA WIP: 	return (0);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_fchmod(unsigned int fd, mode_t mode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	struct file * file;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (fd >= NR_OPEN || !(file = current->filp[fd]))
// TODO WGJA WIP: 		return -EBADF;
// TODO WGJA WIP: 	if (!(inode = file->f_inode))
// TODO WGJA WIP: 		return -ENOENT;
// TODO WGJA WIP: 	if ((current->euid != inode->i_uid) && !suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if (IS_RDONLY(inode))
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	inode->i_mode = (mode & S_IALLUGO) | (inode->i_mode & ~S_IALLUGO);
// TODO WGJA WIP: 	if (!suser() && !in_group_p(inode->i_gid))
// TODO WGJA WIP: 		inode->i_mode &= ~S_ISGID;
// TODO WGJA WIP: 	inode->i_ctime = CURRENT_TIME;
// TODO WGJA WIP: 	inode->i_dirt = 1;
// TODO WGJA WIP: 	return notify_change(NOTIFY_MODE, inode);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_chmod(const char * filename, mode_t mode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = namei(filename,&inode);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if ((current->euid != inode->i_uid) && !suser()) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_RDONLY(inode)) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	inode->i_mode = (mode & S_IALLUGO) | (inode->i_mode & ~S_IALLUGO);
// TODO WGJA WIP: 	if (!suser() && !in_group_p(inode->i_gid))
// TODO WGJA WIP: 		inode->i_mode &= ~S_ISGID;
// TODO WGJA WIP: 	inode->i_ctime = CURRENT_TIME;
// TODO WGJA WIP: 	inode->i_dirt = 1;
// TODO WGJA WIP: 	error = notify_change(NOTIFY_MODE, inode);
// TODO WGJA WIP: 	iput(inode);
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_fchown(unsigned int fd, uid_t user, gid_t group)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	struct file * file;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (fd >= NR_OPEN || !(file = current->filp[fd]))
// TODO WGJA WIP: 		return -EBADF;
// TODO WGJA WIP: 	if (!(inode = file->f_inode))
// TODO WGJA WIP: 		return -ENOENT;
// TODO WGJA WIP: 	if (IS_RDONLY(inode))
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	if (user == (uid_t) -1)
// TODO WGJA WIP: 		user = inode->i_uid;
// TODO WGJA WIP: 	if (group == (gid_t) -1)
// TODO WGJA WIP: 		group = inode->i_gid;
// TODO WGJA WIP: 	if ((current->euid == inode->i_uid && user == inode->i_uid &&
// TODO WGJA WIP: 	     (in_group_p(group) || group == inode->i_gid)) ||
// TODO WGJA WIP: 	    suser()) {
// TODO WGJA WIP: 		inode->i_uid = user;
// TODO WGJA WIP: 		inode->i_gid = group;
// TODO WGJA WIP: 		inode->i_ctime = CURRENT_TIME;
// TODO WGJA WIP: 		inode->i_dirt = 1;
// TODO WGJA WIP: 		return notify_change(NOTIFY_UIDGID, inode);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return -EPERM;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_chown(const char * filename, uid_t user, gid_t group)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = lnamei(filename,&inode);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (IS_RDONLY(inode)) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (user == (uid_t) -1)
// TODO WGJA WIP: 		user = inode->i_uid;
// TODO WGJA WIP: 	if (group == (gid_t) -1)
// TODO WGJA WIP: 		group = inode->i_gid;
// TODO WGJA WIP: 	if ((current->euid == inode->i_uid && user == inode->i_uid &&
// TODO WGJA WIP: 	     (in_group_p(group) || group == inode->i_gid)) ||
// TODO WGJA WIP: 	    suser()) {
// TODO WGJA WIP: 		inode->i_uid = user;
// TODO WGJA WIP: 		inode->i_gid = group;
// TODO WGJA WIP: 		inode->i_ctime = CURRENT_TIME;
// TODO WGJA WIP: 		inode->i_dirt = 1;
// TODO WGJA WIP: 		error = notify_change(NOTIFY_UIDGID, inode);
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	iput(inode);
// TODO WGJA WIP: 	return -EPERM;
// TODO WGJA WIP: }

/*
 * Note that while the flag value (low two bits) for sys_open means:
 *	00 - read-only
 *	01 - write-only
 *	10 - read-write
 *	11 - special
 * it is changed into
 *	00 - no permissions needed
 *	01 - read-permission
 *	10 - write-permission
 *	11 - read-write
 * for the internal routines (ie open_namei()/follow_link() etc). 00 is
 * used by symlinks.
 */
int do_open(const char * filename,int flags,int mode)
{
	struct inode * inode;
	struct file * f;
	int flag,error,fd;
	
	for(fd=0 ; fd<NR_OPEN ; fd++)
		if (!current->filp[fd])
			break;
	if (fd>=NR_OPEN)
		return -EMFILE;
	FD_CLR(fd,&current->close_on_exec);
	f = get_empty_filp();
	if (!f)
		return -ENFILE;
	current->filp[fd] = f;
	f->f_flags = flag = flags;
	f->f_mode = (flag+1) & O_ACCMODE;
	if (f->f_mode)
		flag++;
	if (flag & (O_TRUNC | O_CREAT))
		flag |= 2;
	error = open_namei(filename,flag,mode,&inode,NULL);
	if (error) {
		current->filp[fd]=NULL;
		f->f_count--;
		return error;
	}
	if (flag & O_TRUNC) {
		inode->i_size = 0;
		if (inode->i_op && inode->i_op->truncate)
			inode->i_op->truncate(inode);
		if ((error = notify_change(NOTIFY_SIZE, inode))) {
			iput(inode);
			current->filp[fd] = NULL;
			f->f_count--;
			return error;
		}
		inode->i_dirt = 1;
	}
	f->f_inode = inode;
	f->f_pos = 0;
	f->f_reada = 0;
	f->f_op = NULL;
	if (inode->i_op)
		f->f_op = inode->i_op->default_file_ops;
	if (f->f_op && f->f_op->open) {
		error = f->f_op->open(inode,f);
		if (error) {
			iput(inode);
			f->f_count--;
			current->filp[fd]=NULL;
			return error;
		}
	}
	f->f_flags &= ~(O_CREAT | O_EXCL | O_NOCTTY | O_TRUNC);
	return (fd);
}

extern "C" int sys_open(const char * filename,int flags,int mode)
{
	char * tmp;
	int error;

	error = getname(filename, &tmp);
	if (error)
		return error;
	error = do_open(tmp,flags,mode);
	putname(tmp);
	return error;
}

// TODO WGJA WIP: extern "C" int sys_creat(const char * pathname, int mode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return sys_open(pathname, O_CREAT | O_WRONLY | O_TRUNC, mode);
// TODO WGJA WIP: }

int close_fp(struct file *filp)
{
	struct inode *inode;

	if (filp->f_count == 0) {
		printk("VFS: Close: file count is 0\n");
		return 0;
	}
	inode = filp->f_inode;
	if (inode && S_ISREG(inode->i_mode))
		fcntl_remove_locks(current, filp);
	if (filp->f_count > 1) {
		filp->f_count--;
		return 0;
	}
	if (filp->f_op && filp->f_op->release)
		filp->f_op->release(inode,filp);
	filp->f_count--;
	filp->f_inode = NULL;
	iput(inode);
	return 0;
}

extern "C" int sys_close(unsigned int fd)
{	
	struct file * filp;

	if (fd >= NR_OPEN)
		return -EBADF;
	FD_CLR(fd, &current->close_on_exec);
	if (!(filp = current->filp[fd]))
		return -EBADF;
	current->filp[fd] = NULL;
	return (close_fp (filp));
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This routine simulates a hangup on the tty, to arrange that users
// TODO WGJA WIP:  * are given clean terminals at login time.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_vhangup(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct tty_struct *tty;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	/* See if there is a controlling tty. */
// TODO WGJA WIP: 	if (current->tty < 0)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	tty = TTY_TABLE(MINOR(current->tty));
// TODO WGJA WIP: 	tty_vhangup(tty);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
