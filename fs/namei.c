/*
 *  linux/fs/namei.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * Some corrections by tytso.
 */

#include <asm/segment.h>

#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/stat.h>

#define ACC_MODE(x) ("\000\004\002\006"[(x)&O_ACCMODE])

/*
 * In order to reduce some races, while at the same time doing additional
 * checking and hopefully speeding things up, we copy filenames to the
 * kernel data space before using them..
 *
 * POSIX.1 2.4: an empty pathname is invalid (ENOENT).
 */
int getname(const char * filename, char **result)
{
	int error;
	unsigned long i, page;
	char * tmp, c;

	i = (unsigned long) filename;
	if (!i || i >= TASK_SIZE)
		return -EFAULT;
	i = TASK_SIZE - i;
	error = -EFAULT;
	if (i > PAGE_SIZE) {
		i = PAGE_SIZE;
		error = -ENAMETOOLONG;
	}
	c = get_fs_byte(filename++);
	if (!c)
		return -ENOENT;
	if(!(page = __get_free_page(GFP_KERNEL)))
		return -ENOMEM;
	*result = tmp = (char *) page;
	while (--i) {
		*(tmp++) = c;
		c = get_fs_byte(filename++);
		if (!c) {
			*tmp = '\0';
			return 0;
		}
	}
	free_page(page);
	return error;
}

void putname(char * name)
{
	free_page((unsigned long) name);
}

/*
 *	permission()
 *
 * is used to check for read/write/execute permissions on a file.
 * I don't know if we should look at just the euid or both euid and
 * uid, but that should be easily changed.
 */
int permission(struct inode * inode,int mask)
{
	int mode = inode->i_mode;

/* special case: not even root can read/write a deleted file */
	if (inode->i_dev && !inode->i_nlink)
		return 0;
	else if (inode->i_op && inode->i_op->permission)
		return inode->i_op->permission(inode, mask);
	else if (current->euid == inode->i_uid)
		mode >>= 6;
	else if (in_group_p(inode->i_gid))
		mode >>= 3;
	if (((mode & mask & 0007) == mask) || suser())
		return 1;
	return 0;
}

/*
 * lookup() looks up one part of a pathname, using the fs-dependent
 * routines (currently minix_lookup) for it. It also checks for
 * fathers (pseudo-roots, mount-points)
 */
int lookup(struct inode * dir,const char * name, int len,
	struct inode ** result)
{
	struct super_block * sb;
	int perm;

	*result = NULL;
	if (!dir)
		return -ENOENT;
/* check permissions before traversing mount-points */
	perm = permission(dir,MAY_EXEC);
	if (len==2 && name[0] == '.' && name[1] == '.') {
		if (dir == current->root) {
			*result = dir;
			return 0;
		} else if ((sb = dir->i_sb) && (dir == sb->s_mounted)) {
			sb = dir->i_sb;
			iput(dir);
			dir = sb->s_covered;
			if (!dir)
				return -ENOENT;
			dir->i_count++;
		}
	}
	if (!dir->i_op || !dir->i_op->lookup) {
		iput(dir);
		return -ENOTDIR;
	}
 	if (!perm) {
		iput(dir);
		return -EACCES;
	}
	if (!len) {
		*result = dir;
		return 0;
	}
	return dir->i_op->lookup(dir,name,len,result);
}

int follow_link(struct inode * dir, struct inode * inode,
	int flag, int mode, struct inode ** res_inode)
{
	if (!dir || !inode) {
		iput(dir);
		iput(inode);
		*res_inode = NULL;
		return -ENOENT;
	}
	if (!inode->i_op || !inode->i_op->follow_link) {
		iput(dir);
		*res_inode = inode;
		return 0;
	}
	return inode->i_op->follow_link(dir,inode,flag,mode,res_inode);
}

/*
 *	dir_namei()
 *
 * dir_namei() returns the inode of the directory of the
 * specified name, and the name within that directory.
 */
static int dir_namei(const char * pathname, int * namelen, const char ** name,
	struct inode * base, struct inode ** res_inode)
{
	char c;
	const char * thisname;
	int len,error;
	struct inode * inode;

	*res_inode = NULL;
	if (!base) {
		base = current->pwd;
		base->i_count++;
	}
	if ((c = *pathname) == '/') {
		iput(base);
		base = current->root;
		pathname++;
		base->i_count++;
	}

	while (1) {
		thisname = pathname;
		for(len=0;(c = *(pathname++))&&(c != '/');len++)
			/* nothing */ ;
		if (!c)
			break;
		base->i_count++;
		error = lookup(base,thisname,len,&inode);
		if (error) {
			iput(base);
			return error;
		}
		error = follow_link(base,inode,0,0,&base);
		if (error)
			return error;
	}
	if (!base->i_op || !base->i_op->lookup) {
		iput(base);
		return -ENOTDIR;
	}
	*name = thisname;
	*namelen = len;
	*res_inode = base;
	return 0;
}

// TODO WGJA WIP: static int _namei(const char * pathname, struct inode * base,
// TODO WGJA WIP: 	int follow_links, struct inode ** res_inode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	const char * basename;
// TODO WGJA WIP: 	int namelen,error;
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	*res_inode = NULL;
// TODO WGJA WIP: 	error = dir_namei(pathname,&namelen,&basename,base,&base);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	base->i_count++;	/* lookup uses up base */
// TODO WGJA WIP: 	error = lookup(base,basename,namelen,&inode);
// TODO WGJA WIP: 	if (error) {
// TODO WGJA WIP: 		iput(base);
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (follow_links) {
// TODO WGJA WIP: 		error = follow_link(base,inode,0,0,&inode);
// TODO WGJA WIP: 		if (error)
// TODO WGJA WIP: 			return error;
// TODO WGJA WIP: 	} else
// TODO WGJA WIP: 		iput(base);
// TODO WGJA WIP: 	*res_inode = inode;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int lnamei(const char * pathname, struct inode ** res_inode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	char * tmp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = getname(pathname,&tmp);
// TODO WGJA WIP: 	if (!error) {
// TODO WGJA WIP: 		error = _namei(tmp,NULL,0,res_inode);
// TODO WGJA WIP: 		putname(tmp);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  *	namei()
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * is used by most simple commands to get the inode of a specified name.
// TODO WGJA WIP:  * Open, link etc use their own routines, but this is enough for things
// TODO WGJA WIP:  * like 'chmod' etc.
// TODO WGJA WIP:  */
// TODO WGJA WIP: int namei(const char * pathname, struct inode ** res_inode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	char * tmp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = getname(pathname,&tmp);
// TODO WGJA WIP: 	if (!error) {
// TODO WGJA WIP: 		error = _namei(tmp,NULL,1,res_inode);
// TODO WGJA WIP: 		putname(tmp);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }

/*
 *	open_namei()
 *
 * namei for open - this is in fact almost the whole open-routine.
 *
 * Note that the low bits of "flag" aren't the same asin the open
 * system call - they are 00 - no permissions needed
 *			  01 - read permission needed
 *			  10 - write permission needed
 *			  11 - read/write permissions needed
 * which is a lot more logical, and also allows the "no perm" needed
 * for symlinks (where the permissions are checked later).
 */
int open_namei(const char * pathname, int flag, int mode,
	struct inode ** res_inode, struct inode * base)
{
	const char * basename;
	int namelen,error,i;
	struct inode * dir, *inode;
	struct task_struct ** p;

	mode &= S_IALLUGO & ~current->umask;
	mode |= S_IFREG;
	error = dir_namei(pathname,&namelen,&basename,base,&dir);
	if (error)
		return error;
	if (!namelen) {			/* special case: '/usr/' etc */
		if (flag & 2) {
			iput(dir);
			return -EISDIR;
		}
		/* thanks to Paul Pluzhnikov for noticing this was missing.. */
		if (!permission(dir,ACC_MODE(flag))) {
			iput(dir);
			return -EACCES;
		}
		*res_inode=dir;
		return 0;
	}
	for (i = 0; i < 5; i++) {	/* races... */
		dir->i_count++;		/* lookup eats the dir */
		error = lookup(dir,basename,namelen,&inode);
		if (!error)
			break;
		if (!(flag & O_CREAT)) {
			iput(dir);
			return error;
		}
		if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
			iput(dir);
			return -EACCES;
		}
		if (!dir->i_op || !dir->i_op->create) {
			iput(dir);
			return -EACCES;
		}
		if (IS_RDONLY(dir)) {
			iput(dir);
			return -EROFS;
		}
		dir->i_count++;		/* create eats the dir */
		error = dir->i_op->create(dir,basename,namelen,mode,res_inode);
		if (error != -EEXIST) {
			iput(dir);
			return error;
		}
	}
	if (error) {
		iput(dir);
		return error;
	}
	if ((flag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
		iput(dir);
		iput(inode);
		return -EEXIST;
	}
	error = follow_link(dir,inode,flag,mode,&inode);
	if (error)
		return error;
	if (S_ISDIR(inode->i_mode) && (flag & 2)) {
		iput(inode);
		return -EISDIR;
	}
	if (!permission(inode,ACC_MODE(flag))) {
		iput(inode);
		return -EACCES;
	}
	if (S_ISBLK(inode->i_mode) || S_ISCHR(inode->i_mode)) {
		if (IS_NODEV(inode)) {
			iput(inode);
			return -EACCES;
		}
	} else {
		if (IS_RDONLY(inode) && (flag & 2)) {
			iput(inode);
			return -EROFS;
		}
	}
 	if ((inode->i_count > 1) && (flag & 2)) {
 		for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		        struct vm_area_struct * mpnt;
 			if (!*p)
 				continue;
 			if (inode == (*p)->executable) {
 				iput(inode);
 				return -ETXTBSY;
 			}
			for(mpnt = (*p)->mmap; mpnt; mpnt = mpnt->vm_next) {
				if (inode == mpnt->vm_inode) {
					iput(inode);
					return -ETXTBSY;
				}
			}
 		}
 	}
	*res_inode = inode;
	return 0;
}

// TODO WGJA WIP: int do_mknod(const char * filename, int mode, dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	const char * basename;
// TODO WGJA WIP: 	int namelen, error;
// TODO WGJA WIP: 	struct inode * dir;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	mode &= ~current->umask;
// TODO WGJA WIP: 	error = dir_namei(filename,&namelen,&basename, NULL, &dir);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (!namelen) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -ENOENT;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_RDONLY(dir)) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!dir->i_op || !dir->i_op->mknod) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return dir->i_op->mknod(dir,basename,namelen,mode,dev);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_mknod(const char * filename, int mode, dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	char * tmp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (S_ISDIR(mode)  || (!S_ISFIFO(mode) && !suser()))
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	error = getname(filename,&tmp);
// TODO WGJA WIP: 	if (!error) {
// TODO WGJA WIP: 		error = do_mknod(tmp,mode,dev);
// TODO WGJA WIP: 		putname(tmp);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }

static int do_mkdir(const char * pathname, int mode)
{
	const char * basename;
	int namelen, error;
	struct inode * dir;

	error = dir_namei(pathname,&namelen,&basename,NULL,&dir);
	if (error)
		return error;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (IS_RDONLY(dir)) {
		iput(dir);
		return -EROFS;
	}
	if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
		iput(dir);
		return -EACCES;
	}
	if (!dir->i_op || !dir->i_op->mkdir) {
		iput(dir);
		return -EPERM;
	}
	return dir->i_op->mkdir(dir,basename,namelen,mode);
}

extern "C" int sys_mkdir(const char * pathname, int mode)
{
	int error;
	char * tmp;

	error = getname(pathname,&tmp);
	if (!error) {
		error = do_mkdir(tmp,mode);
		putname(tmp);
	}
	return error;
}

static int do_rmdir(const char * name)
{
	const char * basename;
	int namelen, error;
	struct inode * dir;

	error = dir_namei(name,&namelen,&basename,NULL,&dir);
	if (error)
		return error;
	if (!namelen) {
		iput(dir);
		return -ENOENT;
	}
	if (IS_RDONLY(dir)) {
		iput(dir);
		return -EROFS;
	}
	if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
		iput(dir);
		return -EACCES;
	}
	if (!dir->i_op || !dir->i_op->rmdir) {
		iput(dir);
		return -EPERM;
	}
	return dir->i_op->rmdir(dir,basename,namelen);
}

extern "C" int sys_rmdir(const char * pathname)
{
	int error;
	char * tmp;

	error = getname(pathname,&tmp);
	if (!error) {
		error = do_rmdir(tmp);
		putname(tmp);
	}
	return error;
}

// TODO WGJA WIP: static int do_unlink(const char * name)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	const char * basename;
// TODO WGJA WIP: 	int namelen, error;
// TODO WGJA WIP: 	struct inode * dir;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = dir_namei(name,&namelen,&basename,NULL,&dir);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (!namelen) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_RDONLY(dir)) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!dir->i_op || !dir->i_op->unlink) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return dir->i_op->unlink(dir,basename,namelen);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_unlink(const char * pathname)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	char * tmp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = getname(pathname,&tmp);
// TODO WGJA WIP: 	if (!error) {
// TODO WGJA WIP: 		error = do_unlink(tmp);
// TODO WGJA WIP: 		putname(tmp);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int do_symlink(const char * oldname, const char * newname)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * dir;
// TODO WGJA WIP: 	const char * basename;
// TODO WGJA WIP: 	int namelen, error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = dir_namei(newname,&namelen,&basename,NULL,&dir);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (!namelen) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -ENOENT;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_RDONLY(dir)) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!dir->i_op || !dir->i_op->symlink) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return dir->i_op->symlink(dir,basename,namelen,oldname);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_symlink(const char * oldname, const char * newname)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	char * from, * to;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = getname(oldname,&from);
// TODO WGJA WIP: 	if (!error) {
// TODO WGJA WIP: 		error = getname(newname,&to);
// TODO WGJA WIP: 		if (!error) {
// TODO WGJA WIP: 			error = do_symlink(from,to);
// TODO WGJA WIP: 			putname(to);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		putname(from);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int do_link(struct inode * oldinode, const char * newname)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * dir;
// TODO WGJA WIP: 	const char * basename;
// TODO WGJA WIP: 	int namelen, error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = dir_namei(newname,&namelen,&basename,NULL,&dir);
// TODO WGJA WIP: 	if (error) {
// TODO WGJA WIP: 		iput(oldinode);
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!namelen) {
// TODO WGJA WIP: 		iput(oldinode);
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_RDONLY(dir)) {
// TODO WGJA WIP: 		iput(oldinode);
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (dir->i_dev != oldinode->i_dev) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		iput(oldinode);
// TODO WGJA WIP: 		return -EXDEV;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!permission(dir,MAY_WRITE | MAY_EXEC)) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		iput(oldinode);
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!dir->i_op || !dir->i_op->link) {
// TODO WGJA WIP: 		iput(dir);
// TODO WGJA WIP: 		iput(oldinode);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return dir->i_op->link(oldinode, dir, basename, namelen);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_link(const char * oldname, const char * newname)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	char * to;
// TODO WGJA WIP: 	struct inode * oldinode;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = namei(oldname, &oldinode);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	error = getname(newname,&to);
// TODO WGJA WIP: 	if (!error) {
// TODO WGJA WIP: 		error = do_link(oldinode,to);
// TODO WGJA WIP: 		putname(to);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int do_rename(const char * oldname, const char * newname)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * old_dir, * new_dir;
// TODO WGJA WIP: 	const char * old_base, * new_base;
// TODO WGJA WIP: 	int old_len, new_len, error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = dir_namei(oldname,&old_len,&old_base,NULL,&old_dir);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (!permission(old_dir,MAY_WRITE | MAY_EXEC)) {
// TODO WGJA WIP: 		iput(old_dir);
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!old_len || (old_base[0] == '.' &&
// TODO WGJA WIP: 	    (old_len == 1 || (old_base[1] == '.' &&
// TODO WGJA WIP: 	     old_len == 2)))) {
// TODO WGJA WIP: 		iput(old_dir);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	error = dir_namei(newname,&new_len,&new_base,NULL,&new_dir);
// TODO WGJA WIP: 	if (error) {
// TODO WGJA WIP: 		iput(old_dir);
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!permission(new_dir,MAY_WRITE | MAY_EXEC)) {
// TODO WGJA WIP: 		iput(old_dir);
// TODO WGJA WIP: 		iput(new_dir);
// TODO WGJA WIP: 		return -EACCES;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!new_len || (new_base[0] == '.' &&
// TODO WGJA WIP: 	    (new_len == 1 || (new_base[1] == '.' &&
// TODO WGJA WIP: 	     new_len == 2)))) {
// TODO WGJA WIP: 		iput(old_dir);
// TODO WGJA WIP: 		iput(new_dir);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (new_dir->i_dev != old_dir->i_dev) {
// TODO WGJA WIP: 		iput(old_dir);
// TODO WGJA WIP: 		iput(new_dir);
// TODO WGJA WIP: 		return -EXDEV;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (IS_RDONLY(new_dir) || IS_RDONLY(old_dir)) {
// TODO WGJA WIP: 		iput(old_dir);
// TODO WGJA WIP: 		iput(new_dir);
// TODO WGJA WIP: 		return -EROFS;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!old_dir->i_op || !old_dir->i_op->rename) {
// TODO WGJA WIP: 		iput(old_dir);
// TODO WGJA WIP: 		iput(new_dir);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return old_dir->i_op->rename(old_dir, old_base, old_len, 
// TODO WGJA WIP: 		new_dir, new_base, new_len);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_rename(const char * oldname, const char * newname)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	char * from, * to;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = getname(oldname,&from);
// TODO WGJA WIP: 	if (!error) {
// TODO WGJA WIP: 		error = getname(newname,&to);
// TODO WGJA WIP: 		if (!error) {
// TODO WGJA WIP: 			error = do_rename(from,to);
// TODO WGJA WIP: 			putname(to);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		putname(from);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return error;
// TODO WGJA WIP: }
