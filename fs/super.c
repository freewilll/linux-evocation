/*
 *  linux/fs/super.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * super.c contains code to handle the super-block tables.
 */
#include <linux/config.h>
#include <linux/sched.h>
#include <linux/kernel.h>
// TODO WGJA WIP: #include <linux/stat.h>
#include <linux/errno.h>
#include <linux/string.h>
// TODO WGJA WIP: #include <linux/locks.h>
// TODO WGJA WIP: 
#include <asm/system.h>
#include <asm/segment.h>


/*
 * The definition of file_systems that used to be here is now in
 * filesystems.c.  Now super.c contains no fs specific code.  -- jrs
 */

// TODO WGJA WIP: extern struct file_system_type file_systems[];
// TODO WGJA WIP: extern struct file_operations * blkdev_fops[];
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void wait_for_keypress(void);
// TODO WGJA WIP: extern void fcntl_init_locks(void);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int root_mountflags;
// TODO WGJA WIP: 
// TODO WGJA WIP: struct super_block super_blocks[NR_SUPER];
// TODO WGJA WIP: 
// TODO WGJA WIP: static int do_remount_sb(struct super_block *sb, int flags);

/* this is initialized in init/main.c */
dev_t ROOT_DEV = 0;

// TODO WGJA WIP: struct file_system_type *get_fs_type(char *name)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int a;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!name)
// TODO WGJA WIP: 		return &file_systems[0];
// TODO WGJA WIP: 	for(a = 0 ; file_systems[a].read_super ; a++)
// TODO WGJA WIP: 		if (!strcmp(name,file_systems[a].name))
// TODO WGJA WIP: 			return(&file_systems[a]);
// TODO WGJA WIP: 	return NULL;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void __wait_on_super(struct super_block * sb)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct wait_queue wait = { current, NULL };
// TODO WGJA WIP: 
// TODO WGJA WIP: 	add_wait_queue(&sb->s_wait, &wait);
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 	current->state = TASK_UNINTERRUPTIBLE;
// TODO WGJA WIP: 	if (sb->s_lock) {
// TODO WGJA WIP: 		schedule();
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	remove_wait_queue(&sb->s_wait, &wait);
// TODO WGJA WIP: 	current->state = TASK_RUNNING;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void sync_supers(dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct super_block * sb;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for (sb = super_blocks + 0 ; sb < super_blocks + NR_SUPER ; sb++) {
// TODO WGJA WIP: 		if (!sb->s_dev)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (dev && sb->s_dev != dev)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		wait_on_super(sb);
// TODO WGJA WIP: 		if (!sb->s_dev || !sb->s_dirt)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (dev && (dev != sb->s_dev))
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (sb->s_op && sb->s_op->write_super)
// TODO WGJA WIP: 			sb->s_op->write_super(sb);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static struct super_block * get_super(dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct super_block * s;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!dev)
// TODO WGJA WIP: 		return NULL;
// TODO WGJA WIP: 	s = 0+super_blocks;
// TODO WGJA WIP: 	while (s < NR_SUPER+super_blocks)
// TODO WGJA WIP: 		if (s->s_dev == dev) {
// TODO WGJA WIP: 			wait_on_super(s);
// TODO WGJA WIP: 			if (s->s_dev == dev)
// TODO WGJA WIP: 				return s;
// TODO WGJA WIP: 			s = 0+super_blocks;
// TODO WGJA WIP: 		} else
// TODO WGJA WIP: 			s++;
// TODO WGJA WIP: 	return NULL;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void put_super(dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct super_block * sb;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (dev == ROOT_DEV) {
// TODO WGJA WIP: 		printk("VFS: Root device %d/%d: prepare for armageddon\n",
// TODO WGJA WIP: 							MAJOR(dev), MINOR(dev));
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!(sb = get_super(dev)))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (sb->s_covered) {
// TODO WGJA WIP: 		printk("VFS: Mounted device %d/%d - tssk, tssk\n",
// TODO WGJA WIP: 						MAJOR(dev), MINOR(dev));
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (sb->s_op && sb->s_op->put_super)
// TODO WGJA WIP: 		sb->s_op->put_super(sb);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static struct super_block * read_super(dev_t dev,char *name,int flags,
// TODO WGJA WIP: 				       void *data, int silent)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct super_block * s;
// TODO WGJA WIP: 	struct file_system_type *type;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!dev)
// TODO WGJA WIP: 		return NULL;
// TODO WGJA WIP: 	check_disk_change(dev);
// TODO WGJA WIP: 	s = get_super(dev);
// TODO WGJA WIP: 	if (s)
// TODO WGJA WIP: 		return s;
// TODO WGJA WIP: 	if (!(type = get_fs_type(name))) {
// TODO WGJA WIP: 		printk("VFS: on device %d/%d: get_fs_type(%s) failed\n",
// TODO WGJA WIP: 						MAJOR(dev), MINOR(dev), name);
// TODO WGJA WIP: 		return NULL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	for (s = 0+super_blocks ;; s++) {
// TODO WGJA WIP: 		if (s >= NR_SUPER+super_blocks)
// TODO WGJA WIP: 			return NULL;
// TODO WGJA WIP: 		if (!s->s_dev)
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	s->s_dev = dev;
// TODO WGJA WIP: 	s->s_flags = flags;
// TODO WGJA WIP: 	if (!type->read_super(s,data, silent)) {
// TODO WGJA WIP: 		s->s_dev = 0;
// TODO WGJA WIP: 		return NULL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	s->s_dev = dev;
// TODO WGJA WIP: 	s->s_covered = NULL;
// TODO WGJA WIP: 	s->s_rd_only = 0;
// TODO WGJA WIP: 	s->s_dirt = 0;
// TODO WGJA WIP: 	return s;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Unnamed block devices are dummy devices used by virtual
// TODO WGJA WIP:  * filesystems which don't use real block-devices.  -- jrs
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: static char unnamed_dev_in_use[256];
// TODO WGJA WIP: 
// TODO WGJA WIP: static dev_t get_unnamed_dev(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	static int first_use = 0;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (first_use == 0) {
// TODO WGJA WIP: 		first_use = 1;
// TODO WGJA WIP: 		memset(unnamed_dev_in_use, 0, sizeof(unnamed_dev_in_use));
// TODO WGJA WIP: 		unnamed_dev_in_use[0] = 1; /* minor 0 (nodev) is special */
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	for (i = 0; i < sizeof unnamed_dev_in_use/sizeof unnamed_dev_in_use[0]; i++) {
// TODO WGJA WIP: 		if (!unnamed_dev_in_use[i]) {
// TODO WGJA WIP: 			unnamed_dev_in_use[i] = 1;
// TODO WGJA WIP: 			return (UNNAMED_MAJOR << 8) | i;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static void put_unnamed_dev(dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (!dev)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (!unnamed_dev_in_use[dev]) {
// TODO WGJA WIP: 		printk("VFS: put_unnamed_dev: freeing unused device %d/%d\n",
// TODO WGJA WIP: 							MAJOR(dev), MINOR(dev));
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	unnamed_dev_in_use[dev] = 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int do_umount(dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct super_block * sb;
// TODO WGJA WIP: 	int retval;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (dev==ROOT_DEV) {
// TODO WGJA WIP: 		/* Special case for "unmounting" root.  We just try to remount
// TODO WGJA WIP: 		   it readonly, and sync() the device. */
// TODO WGJA WIP: 		if (!(sb=get_super(dev)))
// TODO WGJA WIP: 			return -ENOENT;
// TODO WGJA WIP: 		if (!(sb->s_flags & MS_RDONLY)) {
// TODO WGJA WIP: 			retval = do_remount_sb(sb, MS_RDONLY);
// TODO WGJA WIP: 			if (retval)
// TODO WGJA WIP: 				return retval;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!(sb=get_super(dev)) || !(sb->s_covered))
// TODO WGJA WIP: 		return -ENOENT;
// TODO WGJA WIP: 	if (!sb->s_covered->i_mount)
// TODO WGJA WIP: 		printk("VFS: umount(%d/%d): mounted inode has i_mount=0\n",
// TODO WGJA WIP: 							MAJOR(dev), MINOR(dev));
// TODO WGJA WIP: 	if (!fs_may_umount(dev, sb->s_mounted))
// TODO WGJA WIP: 		return -EBUSY;
// TODO WGJA WIP: 	sb->s_covered->i_mount=0;
// TODO WGJA WIP: 	iput(sb->s_covered);
// TODO WGJA WIP: 	sb->s_covered = NULL;
// TODO WGJA WIP: 	iput(sb->s_mounted);
// TODO WGJA WIP: 	sb->s_mounted = NULL;
// TODO WGJA WIP: 	if (sb->s_op && sb->s_op->write_super && sb->s_dirt)
// TODO WGJA WIP: 		sb->s_op->write_super(sb);
// TODO WGJA WIP: 	put_super(dev);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Now umount can handle mount points as well as block devices.
// TODO WGJA WIP:  * This is important for filesystems which use unnamed block devices.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * There is a little kludge here with the dummy_inode.  The current
// TODO WGJA WIP:  * vfs release functions only use the r_dev field in the inode so
// TODO WGJA WIP:  * we give them the info they need without using a real inode.
// TODO WGJA WIP:  * If any other fields are ever needed by any block device release
// TODO WGJA WIP:  * functions, they should be faked here.  -- jrs
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_umount(char * name)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	dev_t dev;
// TODO WGJA WIP: 	int retval;
// TODO WGJA WIP: 	struct inode dummy_inode;
// TODO WGJA WIP: 	struct file_operations * fops;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	retval = namei(name,&inode);
// TODO WGJA WIP: 	if (retval) {
// TODO WGJA WIP: 		retval = lnamei(name,&inode);
// TODO WGJA WIP: 		if (retval)
// TODO WGJA WIP: 			return retval;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (S_ISBLK(inode->i_mode)) {
// TODO WGJA WIP: 		dev = inode->i_rdev;
// TODO WGJA WIP: 		if (IS_NODEV(inode)) {
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			return -EACCES;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		if (!inode || !inode->i_sb || inode != inode->i_sb->s_mounted) {
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			return -EINVAL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		dev = inode->i_sb->s_dev;
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		memset(&dummy_inode, 0, sizeof(dummy_inode));
// TODO WGJA WIP: 		dummy_inode.i_rdev = dev;
// TODO WGJA WIP: 		inode = &dummy_inode;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (MAJOR(dev) >= MAX_BLKDEV) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return -ENXIO;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!(retval = do_umount(dev)) && dev != ROOT_DEV) {
// TODO WGJA WIP: 		fops = blkdev_fops[MAJOR(dev)];
// TODO WGJA WIP: 		if (fops && fops->release)
// TODO WGJA WIP: 			fops->release(inode,NULL);
// TODO WGJA WIP: 		if (MAJOR(dev) == UNNAMED_MAJOR)
// TODO WGJA WIP: 			put_unnamed_dev(dev);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (inode != &dummy_inode)
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 	if (retval)
// TODO WGJA WIP: 		return retval;
// TODO WGJA WIP: 	fsync_dev(dev);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * do_mount() does the actual mounting after sys_mount has done the ugly
// TODO WGJA WIP:  * parameter parsing. When enough time has gone by, and everything uses the
// TODO WGJA WIP:  * new mount() parameters, sys_mount() can then be cleaned up.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * We cannot mount a filesystem if it has active, used, or dirty inodes.
// TODO WGJA WIP:  * We also have to flush all inode-data for this device, as the new mount
// TODO WGJA WIP:  * might need new info.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static int do_mount(dev_t dev, const char * dir, char * type, int flags, void * data)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * dir_i;
// TODO WGJA WIP: 	struct super_block * sb;
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	error = namei(dir,&dir_i);
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	if (dir_i->i_count != 1 || dir_i->i_mount) {
// TODO WGJA WIP: 		iput(dir_i);
// TODO WGJA WIP: 		return -EBUSY;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!S_ISDIR(dir_i->i_mode)) {
// TODO WGJA WIP: 		iput(dir_i);
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!fs_may_mount(dev)) {
// TODO WGJA WIP: 		iput(dir_i);
// TODO WGJA WIP: 		return -EBUSY;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	sb = read_super(dev,type,flags,data,0);
// TODO WGJA WIP: 	if (!sb || sb->s_covered) {
// TODO WGJA WIP: 		iput(dir_i);
// TODO WGJA WIP: 		return -EBUSY;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	sb->s_covered = dir_i;
// TODO WGJA WIP: 	dir_i->i_mount = 1;
// TODO WGJA WIP: 	return 0;		/* we don't iput(dir_i) - see umount */
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Alters the mount flags of a mounted file system. Only the mount point
// TODO WGJA WIP:  * is used as a reference - file system type and the device are ignored.
// TODO WGJA WIP:  * FS-specific mount options can't be altered by remounting.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: static int do_remount_sb(struct super_block *sb, int flags)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int retval;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* If we are remounting RDONLY, make sure there are no rw files open */
// TODO WGJA WIP: 	if ((flags & MS_RDONLY) && !(sb->s_flags & MS_RDONLY))
// TODO WGJA WIP: 		if (!fs_may_remount_ro(sb->s_dev))
// TODO WGJA WIP: 			return -EBUSY;
// TODO WGJA WIP: 	if (sb->s_op && sb->s_op->remount_fs) {
// TODO WGJA WIP: 		retval = sb->s_op->remount_fs(sb, &flags);
// TODO WGJA WIP: 		if (retval)
// TODO WGJA WIP: 			return retval;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	sb->s_flags = (sb->s_flags & ~MS_RMT_MASK) |
// TODO WGJA WIP: 		(flags & MS_RMT_MASK);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int do_remount(const char *dir,int flags)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode *dir_i;
// TODO WGJA WIP: 	int retval;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	retval = namei(dir,&dir_i);
// TODO WGJA WIP: 	if (retval)
// TODO WGJA WIP: 		return retval;
// TODO WGJA WIP: 	if (dir_i != dir_i->i_sb->s_mounted) {
// TODO WGJA WIP: 		iput(dir_i);
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	retval = do_remount_sb(dir_i->i_sb, flags);
// TODO WGJA WIP: 	iput(dir_i);
// TODO WGJA WIP: 	return retval;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Flags is a 16-bit value that allows up to 16 non-fs dependent flags to
// TODO WGJA WIP:  * be given to the mount() call (ie: read-only, no-dev, no-suid etc).
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * data is a (void *) that can point to any structure up to PAGE_SIZE-1 bytes, which
// TODO WGJA WIP:  * can contain arbitrary fs-dependent information (or be NULL).
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * NOTE! As old versions of mount() didn't use this setup, the flags has to have
// TODO WGJA WIP:  * a special 16-bit magic number in the hight word: 0xC0ED. If this magic word
// TODO WGJA WIP:  * isn't present, the flags and data info isn't used, as the syscall assumes we
// TODO WGJA WIP:  * are talking to an older version that didn't understand them.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_mount(char * dev_name, char * dir_name, char * type,
// TODO WGJA WIP: 	unsigned long new_flags, void * data)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct file_system_type * fstype;
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	struct file_operations * fops;
// TODO WGJA WIP: 	dev_t dev;
// TODO WGJA WIP: 	int retval;
// TODO WGJA WIP: 	char tmp[100], * t;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 	unsigned long flags = 0;
// TODO WGJA WIP: 	unsigned long page = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if ((new_flags & (MS_MGC_MSK | MS_REMOUNT)) == (MS_MGC_VAL | MS_REMOUNT)) {
// TODO WGJA WIP: 		return do_remount(dir_name,new_flags & ~MS_MGC_MSK & ~MS_REMOUNT);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (type) {
// TODO WGJA WIP: 		for (i = 0 ; i < 100 ; i++) {
// TODO WGJA WIP: 			if (TASK_SIZE <= (unsigned long) type)
// TODO WGJA WIP: 				return -EFAULT;
// TODO WGJA WIP: 			if (!(tmp[i] = get_fs_byte(type++)))
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		t = tmp;
// TODO WGJA WIP: 	} else
// TODO WGJA WIP: 		t = NULL;
// TODO WGJA WIP: 	if (!(fstype = get_fs_type(t)))
// TODO WGJA WIP: 		return -ENODEV;
// TODO WGJA WIP: 	t = fstype->name;
// TODO WGJA WIP: 	if (fstype->requires_dev) {
// TODO WGJA WIP: 		retval = namei(dev_name,&inode);
// TODO WGJA WIP: 		if (retval)
// TODO WGJA WIP: 			return retval;
// TODO WGJA WIP: 		if (!S_ISBLK(inode->i_mode)) {
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			return -ENOTBLK;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (IS_NODEV(inode)) {
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			return -EACCES;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		dev = inode->i_rdev;
// TODO WGJA WIP: 		if (MAJOR(dev) >= MAX_BLKDEV) {
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			return -ENXIO;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		if (!(dev = get_unnamed_dev()))
// TODO WGJA WIP: 			return -EMFILE;
// TODO WGJA WIP: 		inode = NULL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	fops = blkdev_fops[MAJOR(dev)];
// TODO WGJA WIP: 	if (fops && fops->open) {
// TODO WGJA WIP: 		retval = fops->open(inode,NULL);
// TODO WGJA WIP: 		if (retval) {
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			return retval;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if ((new_flags & MS_MGC_MSK) == MS_MGC_VAL) {
// TODO WGJA WIP: 		flags = new_flags & ~MS_MGC_MSK;
// TODO WGJA WIP: 		if (data) {
// TODO WGJA WIP: 			if ((unsigned long) data >= TASK_SIZE) {
// TODO WGJA WIP: 				iput(inode);
// TODO WGJA WIP: 				return -EFAULT;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			if (!(page = __get_free_page(GFP_KERNEL))) {
// TODO WGJA WIP: 				iput(inode);
// TODO WGJA WIP: 				return -ENOMEM;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			i = TASK_SIZE - (unsigned long) data;
// TODO WGJA WIP: 			if ((unsigned long) i >= PAGE_SIZE)
// TODO WGJA WIP: 				i = PAGE_SIZE-1;
// TODO WGJA WIP: 			memcpy_fromfs((void *) page,data,i);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	retval = do_mount(dev,dir_name,t,flags,(void *) page);
// TODO WGJA WIP: 	free_page(page);
// TODO WGJA WIP: 	if (retval && fops && fops->release)
// TODO WGJA WIP: 		fops->release(inode,NULL);
// TODO WGJA WIP: 	iput(inode);
// TODO WGJA WIP: 	return retval;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void mount_root(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct file_system_type * fs_type;
// TODO WGJA WIP: 	struct super_block * sb;
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	memset(super_blocks, 0, sizeof(super_blocks));
// TODO WGJA WIP: 	fcntl_init_locks();
// TODO WGJA WIP: 	if (MAJOR(ROOT_DEV) == 2) {
// TODO WGJA WIP: 		printk("VFS: Insert root floppy and press ENTER");
// TODO WGJA WIP: 		wait_for_keypress();
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	for (fs_type = file_systems; fs_type->read_super; fs_type++) {
// TODO WGJA WIP: 		if (!fs_type->requires_dev)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		sb = read_super(ROOT_DEV,fs_type->name,root_mountflags,NULL,1);
// TODO WGJA WIP: 		if (sb) {
// TODO WGJA WIP: 			inode = sb->s_mounted;
// TODO WGJA WIP: 			inode->i_count += 3 ;	/* NOTE! it is logically used 4 times, not 1 */
// TODO WGJA WIP: 			sb->s_covered = inode;
// TODO WGJA WIP: 			sb->s_flags = root_mountflags;
// TODO WGJA WIP: 			current->pwd = inode;
// TODO WGJA WIP: 			current->root = inode;
// TODO WGJA WIP: 			printk ("VFS: Mounted root (%s filesystem)%s.\n",
// TODO WGJA WIP: 				fs_type->name,
// TODO WGJA WIP: 				(sb->s_flags & MS_RDONLY) ? " readonly" : "");
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	panic("VFS: Unable to mount root");
// TODO WGJA WIP: }
