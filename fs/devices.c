/*
 *  linux/fs/devices.c
 *
 * (C) 1993 Matthias Urlichs -- collected common code and tables.
 * 
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

// TODO WGJA WIP: #include <linux/fs.h>
#include <linux/string.h>
#include <linux/sched.h>
// TODO WGJA WIP: #include <linux/ext_fs.h>
// TODO WGJA WIP: #include <linux/tty.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/errno.h>

struct file_operations * chrdev_fops[MAX_CHRDEV] = {
	NULL,
};

struct file_operations * blkdev_fops[MAX_BLKDEV] = {
	NULL,
};

int register_chrdev(unsigned int major, const char * name, struct file_operations *fops)
{
	if (major >= MAX_CHRDEV)
		return -EINVAL;
	if (chrdev_fops[major])
		return -EBUSY;
	chrdev_fops[major] = fops;
	return 0;
}

int register_blkdev(unsigned int major, const char * name, struct file_operations *fops)
{
	if (major >= MAX_BLKDEV)
		return -EINVAL;
	if (blkdev_fops[major])
		return -EBUSY;
	blkdev_fops[major] = fops;
	return 0;
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Called every time a block special file is opened
// TODO WGJA WIP:  */
// TODO WGJA WIP: int blkdev_open(struct inode * inode, struct file * filp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	i = MAJOR(inode->i_rdev);
// TODO WGJA WIP: 	if (i >= MAX_BLKDEV || !blkdev_fops[i])
// TODO WGJA WIP: 		return -ENODEV;
// TODO WGJA WIP: 	filp->f_op = blkdev_fops[i];
// TODO WGJA WIP: 	if (filp->f_op->open)
// TODO WGJA WIP: 		return filp->f_op->open(inode,filp);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }	
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Dummy default file-operations: the only thing this does
// TODO WGJA WIP:  * is contain the open that then fills in the correct operations
// TODO WGJA WIP:  * depending on the special file...
// TODO WGJA WIP:  */
// TODO WGJA WIP: struct file_operations def_blk_fops = {
// TODO WGJA WIP: 	NULL,		/* lseek */
// TODO WGJA WIP: 	NULL,		/* read */
// TODO WGJA WIP: 	NULL,		/* write */
// TODO WGJA WIP: 	NULL,		/* readdir */
// TODO WGJA WIP: 	NULL,		/* select */
// TODO WGJA WIP: 	NULL,		/* ioctl */
// TODO WGJA WIP: 	NULL,		/* mmap */
// TODO WGJA WIP: 	blkdev_open,	/* open */
// TODO WGJA WIP: 	NULL,		/* release */
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: struct inode_operations blkdev_inode_operations = {
// TODO WGJA WIP: 	&def_blk_fops,		/* default file operations */
// TODO WGJA WIP: 	NULL,			/* create */
// TODO WGJA WIP: 	NULL,			/* lookup */
// TODO WGJA WIP: 	NULL,			/* link */
// TODO WGJA WIP: 	NULL,			/* unlink */
// TODO WGJA WIP: 	NULL,			/* symlink */
// TODO WGJA WIP: 	NULL,			/* mkdir */
// TODO WGJA WIP: 	NULL,			/* rmdir */
// TODO WGJA WIP: 	NULL,			/* mknod */
// TODO WGJA WIP: 	NULL,			/* rename */
// TODO WGJA WIP: 	NULL,			/* readlink */
// TODO WGJA WIP: 	NULL,			/* follow_link */
// TODO WGJA WIP: 	NULL,			/* bmap */
// TODO WGJA WIP: 	NULL,			/* truncate */
// TODO WGJA WIP: 	NULL			/* permission */
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Called every time a character special file is opened
// TODO WGJA WIP:  */
// TODO WGJA WIP: int chrdev_open(struct inode * inode, struct file * filp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	i = MAJOR(inode->i_rdev);
// TODO WGJA WIP: 	if (i >= MAX_CHRDEV || !chrdev_fops[i])
// TODO WGJA WIP: 		return -ENODEV;
// TODO WGJA WIP: 	filp->f_op = chrdev_fops[i];
// TODO WGJA WIP: 	if (filp->f_op->open)
// TODO WGJA WIP: 		return filp->f_op->open(inode,filp);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Dummy default file-operations: the only thing this does
// TODO WGJA WIP:  * is contain the open that then fills in the correct operations
// TODO WGJA WIP:  * depending on the special file...
// TODO WGJA WIP:  */
// TODO WGJA WIP: struct file_operations def_chr_fops = {
// TODO WGJA WIP: 	NULL,		/* lseek */
// TODO WGJA WIP: 	NULL,		/* read */
// TODO WGJA WIP: 	NULL,		/* write */
// TODO WGJA WIP: 	NULL,		/* readdir */
// TODO WGJA WIP: 	NULL,		/* select */
// TODO WGJA WIP: 	NULL,		/* ioctl */
// TODO WGJA WIP: 	NULL,		/* mmap */
// TODO WGJA WIP: 	chrdev_open,	/* open */
// TODO WGJA WIP: 	NULL,		/* release */
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: struct inode_operations chrdev_inode_operations = {
// TODO WGJA WIP: 	&def_chr_fops,		/* default file operations */
// TODO WGJA WIP: 	NULL,			/* create */
// TODO WGJA WIP: 	NULL,			/* lookup */
// TODO WGJA WIP: 	NULL,			/* link */
// TODO WGJA WIP: 	NULL,			/* unlink */
// TODO WGJA WIP: 	NULL,			/* symlink */
// TODO WGJA WIP: 	NULL,			/* mkdir */
// TODO WGJA WIP: 	NULL,			/* rmdir */
// TODO WGJA WIP: 	NULL,			/* mknod */
// TODO WGJA WIP: 	NULL,			/* rename */
// TODO WGJA WIP: 	NULL,			/* readlink */
// TODO WGJA WIP: 	NULL,			/* follow_link */
// TODO WGJA WIP: 	NULL,			/* bmap */
// TODO WGJA WIP: 	NULL,			/* truncate */
// TODO WGJA WIP: 	NULL			/* permission */
// TODO WGJA WIP: };
