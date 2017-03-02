#pragma GCC diagnostic ignored "-fpermissive"
/*
 *  linux/fs/inode.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/string.h>

#include <asm/system.h>

static struct inode * hash_table[NR_IHASH];
static struct inode * first_inode;
// TODO WGJA WIP: static struct wait_queue * inode_wait = NULL;
static int nr_inodes = 0, nr_free_inodes = 0;

static inline int const hashfn(dev_t dev, unsigned int i)
{
	return (dev ^ i) % NR_IHASH;
}

static inline struct inode ** const hash(dev_t dev, int i)
{
	return hash_table + hashfn(dev, i);
}

static void insert_inode_free(struct inode *inode)
{
	inode->i_next = first_inode;
	inode->i_prev = first_inode->i_prev;
	inode->i_next->i_prev = inode;
	inode->i_prev->i_next = inode;
	first_inode = inode;
}

static void remove_inode_free(struct inode *inode)
{
	if (first_inode == inode)
		first_inode = first_inode->i_next;
	if (inode->i_next)
		inode->i_next->i_prev = inode->i_prev;
	if (inode->i_prev)
		inode->i_prev->i_next = inode->i_next;
	inode->i_next = inode->i_prev = NULL;
}

// TODO WGJA WIP: void insert_inode_hash(struct inode *inode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode **h;
// TODO WGJA WIP: 	h = hash(inode->i_dev, inode->i_ino);
// TODO WGJA WIP: 
// TODO WGJA WIP: 	inode->i_hash_next = *h;
// TODO WGJA WIP: 	inode->i_hash_prev = NULL;
// TODO WGJA WIP: 	if (inode->i_hash_next)
// TODO WGJA WIP: 		inode->i_hash_next->i_hash_prev = inode;
// TODO WGJA WIP: 	*h = inode;
// TODO WGJA WIP: }

static void remove_inode_hash(struct inode *inode)
{
	struct inode **h;
	h = hash(inode->i_dev, inode->i_ino);

	if (*h == inode)
		*h = inode->i_hash_next;
	if (inode->i_hash_next)
		inode->i_hash_next->i_hash_prev = inode->i_hash_prev;
	if (inode->i_hash_prev)
		inode->i_hash_prev->i_hash_next = inode->i_hash_next;
	inode->i_hash_prev = inode->i_hash_next = NULL;
}

// TODO WGJA WIP: static void put_last_free(struct inode *inode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	remove_inode_free(inode);
// TODO WGJA WIP: 	inode->i_prev = first_inode->i_prev;
// TODO WGJA WIP: 	inode->i_prev->i_next = inode;
// TODO WGJA WIP: 	inode->i_next = first_inode;
// TODO WGJA WIP: 	inode->i_next->i_prev = inode;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void grow_inodes(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if(!(inode = (struct inode*) get_free_page(GFP_KERNEL)))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	i=PAGE_SIZE / sizeof(struct inode);
// TODO WGJA WIP: 	nr_inodes += i;
// TODO WGJA WIP: 	nr_free_inodes += i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!first_inode)
// TODO WGJA WIP: 		inode->i_next = inode->i_prev = first_inode = inode++, i--;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for ( ; i ; i-- )
// TODO WGJA WIP: 		insert_inode_free(inode++);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: unsigned long inode_init(unsigned long start, unsigned long end)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	memset(hash_table, 0, sizeof(hash_table));
// TODO WGJA WIP: 	first_inode = NULL;
// TODO WGJA WIP: 	return start;
// TODO WGJA WIP: }

static void __wait_on_inode(struct inode *);

static inline void wait_on_inode(struct inode * inode)
{
	if (inode->i_lock)
		__wait_on_inode(inode);
}

static inline void lock_inode(struct inode * inode)
{
	wait_on_inode(inode);
	inode->i_lock = 1;
}

static inline void unlock_inode(struct inode * inode)
{
	inode->i_lock = 0;
	wake_up(&inode->i_wait);
}

/*
 * Note that we don't want to disturb any wait-queues when we discard
 * an inode.
 *
 * Argghh. Got bitten by a gcc problem with inlining: no way to tell
 * the compiler that the inline asm function 'memset' changes 'inode'.
 * I've been searching for the bug for days, and was getting desperate.
 * Finally looked at the assembler output... Grrr.
 *
 * The solution is the weird use of 'volatile'. Ho humm. Have to report
 * it to the gcc lists, and hope we can do this more cleanly some day..
 */
void clear_inode(struct inode * inode)
{
	struct wait_queue * wait;

	wait_on_inode(inode);
	remove_inode_hash(inode);
	remove_inode_free(inode);
	wait = ((volatile struct inode *) inode)->i_wait;
	if (inode->i_count)
		nr_free_inodes++;
	memset(inode,0,sizeof(*inode));
	((volatile struct inode *) inode)->i_wait = wait;
	insert_inode_free(inode);
}

// TODO WGJA WIP: int fs_may_mount(dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode, * next;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	next = first_inode;
// TODO WGJA WIP: 	for (i = nr_inodes ; i > 0 ; i--) {
// TODO WGJA WIP: 		inode = next;
// TODO WGJA WIP: 		next = inode->i_next;	/* clear_inode() changes the queues.. */
// TODO WGJA WIP: 		if (inode->i_dev != dev)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (inode->i_count || inode->i_dirt || inode->i_lock)
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		clear_inode(inode);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int fs_may_umount(dev_t dev, struct inode * mount_root)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	inode = first_inode;
// TODO WGJA WIP: 	for (i=0 ; i < nr_inodes ; i++, inode = inode->i_next) {
// TODO WGJA WIP: 		if (inode->i_dev != dev || !inode->i_count)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (inode == mount_root && inode->i_count == 1)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int fs_may_remount_ro(dev_t dev)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct file * file;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* Check that no files are currently opened for writing. */
// TODO WGJA WIP: 	for (file = first_file, i=0; i<nr_files; i++, file=file->f_next) {
// TODO WGJA WIP: 		if (!file->f_count || !file->f_inode ||
// TODO WGJA WIP: 		    file->f_inode->i_dev != dev)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (S_ISREG(file->f_inode->i_mode) && (file->f_mode & 2))
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }

static void write_inode(struct inode * inode)
{
	if (!inode->i_dirt)
		return;
	wait_on_inode(inode);
	if (!inode->i_dirt)
		return;
	if (!inode->i_sb || !inode->i_sb->s_op || !inode->i_sb->s_op->write_inode) {
		inode->i_dirt = 0;
		return;
	}
	inode->i_lock = 1;	
	inode->i_sb->s_op->write_inode(inode);
	unlock_inode(inode);
}

// TODO WGJA WIP: static void read_inode(struct inode * inode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	lock_inode(inode);
// TODO WGJA WIP: 	if (inode->i_sb && inode->i_sb->s_op && inode->i_sb->s_op->read_inode)
// TODO WGJA WIP: 		inode->i_sb->s_op->read_inode(inode);
// TODO WGJA WIP: 	unlock_inode(inode);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * notify_change is called for inode-changing operations such as
// TODO WGJA WIP:  * chown, chmod, utime, and truncate.  It is guaranteed (unlike
// TODO WGJA WIP:  * write_inode) to be called from the context of the user requesting
// TODO WGJA WIP:  * the change.  It is not called for ordinary access-time updates.
// TODO WGJA WIP:  * NFS uses this to get the authentication correct.  -- jrs
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: int notify_change(int flags, struct inode * inode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (inode->i_sb && inode->i_sb->s_op  &&
// TODO WGJA WIP: 	    inode->i_sb->s_op->notify_change)
// TODO WGJA WIP: 		return inode->i_sb->s_op->notify_change(flags, inode);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * bmap is needed for demand-loading and paging: if this function
// TODO WGJA WIP:  * doesn't exist for a filesystem, then those things are impossible:
// TODO WGJA WIP:  * executables cannot be run from the filesystem etc...
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * This isn't as bad as it sounds: the read-routines might still work,
// TODO WGJA WIP:  * so the filesystem would be otherwise ok (for example, you might have
// TODO WGJA WIP:  * a DOS filesystem, which doesn't lend itself to bmap very well, but
// TODO WGJA WIP:  * you could still transfer files to/from the filesystem)
// TODO WGJA WIP:  */
// TODO WGJA WIP: int bmap(struct inode * inode, int block)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (inode->i_op && inode->i_op->bmap)
// TODO WGJA WIP: 		return inode->i_op->bmap(inode,block);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }

void invalidate_inodes(dev_t dev)
{
	struct inode * inode, * next;
	int i;

	next = first_inode;
	for(i = nr_inodes ; i > 0 ; i--) {
		inode = next;
		next = inode->i_next;		/* clear_inode() changes the queues.. */
		if (inode->i_dev != dev)
			continue;
		if (inode->i_count || inode->i_dirt || inode->i_lock) {
			printk("VFS: inode busy on removed device %d/%d\n", MAJOR(dev), MINOR(dev));
			continue;
		}
		clear_inode(inode);
	}
}

void sync_inodes(dev_t dev)
{
	int i;
	struct inode * inode;

	inode = first_inode;
	for(i = 0; i < nr_inodes*2; i++, inode = inode->i_next) {
		if (dev && inode->i_dev != dev)
			continue;
		wait_on_inode(inode);
		if (inode->i_dirt)
			write_inode(inode);
	}
}

// TODO WGJA WIP: void iput(struct inode * inode)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (!inode)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	wait_on_inode(inode);
// TODO WGJA WIP: 	if (!inode->i_count) {
// TODO WGJA WIP: 		printk("VFS: iput: trying to free free inode\n");
// TODO WGJA WIP: 		printk("VFS: device %d/%d, inode %d, mode=0%07o\n",
// TODO WGJA WIP: 			MAJOR(inode->i_rdev), MINOR(inode->i_rdev),
// TODO WGJA WIP: 					inode->i_ino, inode->i_mode);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (inode->i_pipe) {
// TODO WGJA WIP: 		wake_up(&PIPE_READ_WAIT(*inode));
// TODO WGJA WIP: 		wake_up(&PIPE_WRITE_WAIT(*inode));
// TODO WGJA WIP: 	}
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 	if (inode->i_count>1) {
// TODO WGJA WIP: 		inode->i_count--;
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	wake_up(&inode_wait);
// TODO WGJA WIP: 	if (inode->i_pipe) {
// TODO WGJA WIP: 		unsigned long page = (unsigned long) PIPE_BASE(*inode);
// TODO WGJA WIP: 		PIPE_BASE(*inode) = NULL;
// TODO WGJA WIP: 		free_page(page);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (inode->i_sb && inode->i_sb->s_op && inode->i_sb->s_op->put_inode) {
// TODO WGJA WIP: 		inode->i_sb->s_op->put_inode(inode);
// TODO WGJA WIP: 		if (!inode->i_nlink)
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (inode->i_dirt) {
// TODO WGJA WIP: 		write_inode(inode);	/* we can sleep - so do again */
// TODO WGJA WIP: 		wait_on_inode(inode);
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	inode->i_count--;
// TODO WGJA WIP: 	nr_free_inodes++;
// TODO WGJA WIP: 	return;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: struct inode * get_empty_inode(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode, * best;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (nr_inodes < NR_INODE && nr_free_inodes < (nr_inodes >> 2))
// TODO WGJA WIP: 		grow_inodes();
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 	inode = first_inode;
// TODO WGJA WIP: 	best = NULL;
// TODO WGJA WIP: 	for (i = 0; i<nr_inodes; inode = inode->i_next, i++) {
// TODO WGJA WIP: 		if (!inode->i_count) {
// TODO WGJA WIP: 			if (!best)
// TODO WGJA WIP: 				best = inode;
// TODO WGJA WIP: 			if (!inode->i_dirt && !inode->i_lock) {
// TODO WGJA WIP: 				best = inode;
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!best || best->i_dirt || best->i_lock)
// TODO WGJA WIP: 		if (nr_inodes < NR_INODE) {
// TODO WGJA WIP: 			grow_inodes();
// TODO WGJA WIP: 			goto repeat;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	inode = best;
// TODO WGJA WIP: 	if (!inode) {
// TODO WGJA WIP: 		printk("VFS: No free inodes - contact Linus\n");
// TODO WGJA WIP: 		sleep_on(&inode_wait);
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (inode->i_lock) {
// TODO WGJA WIP: 		wait_on_inode(inode);
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (inode->i_dirt) {
// TODO WGJA WIP: 		write_inode(inode);
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (inode->i_count)
// TODO WGJA WIP: 		goto repeat;
// TODO WGJA WIP: 	clear_inode(inode);
// TODO WGJA WIP: 	inode->i_count = 1;
// TODO WGJA WIP: 	inode->i_nlink = 1;
// TODO WGJA WIP: 	nr_free_inodes--;
// TODO WGJA WIP: 	if (nr_free_inodes < 0) {
// TODO WGJA WIP: 		printk ("VFS: get_empty_inode: bad free inode count.\n");
// TODO WGJA WIP: 		nr_free_inodes = 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return inode;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: struct inode * get_pipe_inode(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	extern struct inode_operations pipe_inode_operations;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!(inode = get_empty_inode()))
// TODO WGJA WIP: 		return NULL;
// TODO WGJA WIP: 	if (!(PIPE_BASE(*inode) = (char*) __get_free_page(GFP_USER))) {
// TODO WGJA WIP: 		iput(inode);
// TODO WGJA WIP: 		return NULL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	inode->i_op = &pipe_inode_operations;
// TODO WGJA WIP: 	inode->i_count = 2;	/* sum of readers/writers */
// TODO WGJA WIP: 	PIPE_READ_WAIT(*inode) = PIPE_WRITE_WAIT(*inode) = NULL;
// TODO WGJA WIP: 	PIPE_HEAD(*inode) = PIPE_TAIL(*inode) = 0;
// TODO WGJA WIP: 	PIPE_RD_OPENERS(*inode) = PIPE_WR_OPENERS(*inode) = 0;
// TODO WGJA WIP: 	PIPE_READERS(*inode) = PIPE_WRITERS(*inode) = 1;
// TODO WGJA WIP: 	inode->i_pipe = 1;
// TODO WGJA WIP: 	inode->i_mode |= S_IFIFO | S_IRUSR | S_IWUSR;
// TODO WGJA WIP: 	inode->i_uid = current->euid;
// TODO WGJA WIP: 	inode->i_gid = current->egid;
// TODO WGJA WIP: 	inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
// TODO WGJA WIP: 	return inode;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: struct inode * iget(struct super_block * sb,int nr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode, * empty;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!sb)
// TODO WGJA WIP: 		panic("VFS: iget with sb==NULL");
// TODO WGJA WIP: 	empty = get_empty_inode();
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 	inode = *(hash(sb->s_dev,nr));
// TODO WGJA WIP: 	while (inode) {
// TODO WGJA WIP: 		if (inode->i_dev != sb->s_dev || inode->i_ino != nr) {
// TODO WGJA WIP: 			inode = inode->i_hash_next;
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		wait_on_inode(inode);
// TODO WGJA WIP: 		if (inode->i_dev != sb->s_dev || inode->i_ino != nr)
// TODO WGJA WIP: 			goto repeat;
// TODO WGJA WIP: 		if (!inode->i_count)
// TODO WGJA WIP: 			nr_free_inodes--;
// TODO WGJA WIP: 		inode->i_count++;
// TODO WGJA WIP: 		if (inode->i_mount) {
// TODO WGJA WIP: 			int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 			for (i = 0 ; i<NR_SUPER ; i++)
// TODO WGJA WIP: 				if (super_blocks[i].s_covered==inode)
// TODO WGJA WIP: 					break;
// TODO WGJA WIP: 			if (i >= NR_SUPER) {
// TODO WGJA WIP: 				printk("VFS: Mounted inode hasn't got sb\n");
// TODO WGJA WIP: 				if (empty)
// TODO WGJA WIP: 					iput(empty);
// TODO WGJA WIP: 				return inode;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			iput(inode);
// TODO WGJA WIP: 			if (!(inode = super_blocks[i].s_mounted))
// TODO WGJA WIP: 				printk("VFS: Mounted device %d/%d has no rootinode\n",
// TODO WGJA WIP: 					MAJOR(inode->i_dev), MINOR(inode->i_dev));
// TODO WGJA WIP: 			else {
// TODO WGJA WIP: 				if (!inode->i_count)
// TODO WGJA WIP: 					nr_free_inodes--;
// TODO WGJA WIP: 				inode->i_count++;
// TODO WGJA WIP: 				wait_on_inode(inode);
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (empty)
// TODO WGJA WIP: 			iput(empty);
// TODO WGJA WIP: 		return inode;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!empty)
// TODO WGJA WIP: 		return (NULL);
// TODO WGJA WIP: 	inode = empty;
// TODO WGJA WIP: 	inode->i_sb = sb;
// TODO WGJA WIP: 	inode->i_dev = sb->s_dev;
// TODO WGJA WIP: 	inode->i_ino = nr;
// TODO WGJA WIP: 	inode->i_flags = sb->s_flags;
// TODO WGJA WIP: 	put_last_free(inode);
// TODO WGJA WIP: 	insert_inode_hash(inode);
// TODO WGJA WIP: 	read_inode(inode);
// TODO WGJA WIP: 	return inode;
// TODO WGJA WIP: }

/*
 * The "new" scheduling primitives (new as of 0.97 or so) allow this to
 * be done without disabling interrupts (other than in the actual queue
 * updating things: only a couple of 386 instructions). This should be
 * much better for interrupt latency.
 */
static void __wait_on_inode(struct inode * inode)
{
	struct wait_queue wait = { current, NULL };

	add_wait_queue(&inode->i_wait, &wait);
repeat:
	current->state = TASK_UNINTERRUPTIBLE;
	if (inode->i_lock) {
		schedule();
		goto repeat;
	}
	remove_wait_queue(&inode->i_wait, &wait);
	current->state = TASK_RUNNING;
}
