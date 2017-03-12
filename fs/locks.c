/*
 *  linux/fs/locks.c
 *
 *  Provide support for fcntl()'s F_GETLK, F_SETLK, and F_SETLKW calls.
 *  Doug Evans, 92Aug07, dje@sspiff.uucp.
 *
 * FIXME: two things aren't handled yet:
 *	- deadlock detection/avoidance (of dubious merit, but since it's in
 *	  the definition, I guess it should be provided eventually)
 *	- mandatory locks (requires lots of changes elsewhere)
 *
 *  Edited by Kai Petzke, wpp@marie.physik.tu-berlin.de
 */

#include <asm/segment.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <linux/fcntl.h>

// TODO WGJA WIP: #define OFFSET_MAX	((off_t)0x7fffffff)	/* FIXME: move elsewhere? */
// TODO WGJA WIP: 
// TODO WGJA WIP: static int copy_flock(struct file *filp, struct file_lock *fl, struct flock *l);
// TODO WGJA WIP: static int conflict(struct file_lock *caller_fl, struct file_lock *sys_fl);
// TODO WGJA WIP: static int overlap(struct file_lock *fl1, struct file_lock *fl2);
// TODO WGJA WIP: static int lock_it(struct file *filp, struct file_lock *caller);
// TODO WGJA WIP: static struct file_lock *alloc_lock(struct file_lock **pos, struct file_lock *fl);
static void free_lock(struct file_lock **fl);

static struct file_lock file_lock_table[NR_FILE_LOCKS];
static struct file_lock *file_lock_free_list;

/*
 * Called at boot time to initialize the lock table ...
 */

void fcntl_init_locks(void)
{
	struct file_lock *fl;

	for (fl = &file_lock_table[0]; fl < file_lock_table + NR_FILE_LOCKS - 1; fl++) {
		fl->fl_next = fl + 1;
		fl->fl_owner = NULL;
	}
	file_lock_table[NR_FILE_LOCKS - 1].fl_next = NULL;
	file_lock_table[NR_FILE_LOCKS - 1].fl_owner = NULL;
	file_lock_free_list = &file_lock_table[0];
}

// TODO WGJA WIP: int fcntl_getlk(unsigned int fd, struct flock *l)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	struct flock flock;
// TODO WGJA WIP: 	struct file *filp;
// TODO WGJA WIP: 	struct file_lock *fl,file_lock;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (fd >= NR_OPEN || !(filp = current->filp[fd]))
// TODO WGJA WIP: 		return -EBADF;
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE,l, sizeof(*l));
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	memcpy_fromfs(&flock, l, sizeof(flock));
// TODO WGJA WIP: 	if (flock.l_type == F_UNLCK)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	if (!copy_flock(filp, &file_lock, &flock))
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for (fl = filp->f_inode->i_flock; fl != NULL; fl = fl->fl_next) {
// TODO WGJA WIP: 		if (conflict(&file_lock, fl)) {
// TODO WGJA WIP: 			flock.l_pid = fl->fl_owner->pid;
// TODO WGJA WIP: 			flock.l_start = fl->fl_start;
// TODO WGJA WIP: 			flock.l_len = fl->fl_end == OFFSET_MAX ? 0 :
// TODO WGJA WIP: 				fl->fl_end - fl->fl_start + 1;
// TODO WGJA WIP: 			flock.l_whence = fl->fl_whence;
// TODO WGJA WIP: 			flock.l_type = fl->fl_type;
// TODO WGJA WIP: 			memcpy_tofs(l, &flock, sizeof(flock));
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	flock.l_type = F_UNLCK;			/* no conflict found */
// TODO WGJA WIP: 	memcpy_tofs(l, &flock, sizeof(flock));
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This function implements both F_SETLK and F_SETLKW.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: int fcntl_setlk(unsigned int fd, unsigned int cmd, struct flock *l)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int error;
// TODO WGJA WIP: 	struct file *filp;
// TODO WGJA WIP: 	struct file_lock *fl,file_lock;
// TODO WGJA WIP: 	struct flock flock;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Get arguments and validate them ...
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (fd >= NR_OPEN || !(filp = current->filp[fd]))
// TODO WGJA WIP: 		return -EBADF;
// TODO WGJA WIP: 	error = verify_area(VERIFY_WRITE, l, sizeof(*l));
// TODO WGJA WIP: 	if (error)
// TODO WGJA WIP: 		return error;
// TODO WGJA WIP: 	memcpy_fromfs(&flock, l, sizeof(flock));
// TODO WGJA WIP: 	if (!copy_flock(filp, &file_lock, &flock))
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	switch (file_lock.fl_type) {
// TODO WGJA WIP: 	case F_RDLCK :
// TODO WGJA WIP: 		if (!(filp->f_mode & 1))
// TODO WGJA WIP: 			return -EBADF;
// TODO WGJA WIP: 		break;
// TODO WGJA WIP: 	case F_WRLCK :
// TODO WGJA WIP: 		if (!(filp->f_mode & 2))
// TODO WGJA WIP: 			return -EBADF;
// TODO WGJA WIP: 		break;
// TODO WGJA WIP: 	case F_SHLCK :
// TODO WGJA WIP: 		if (!(filp->f_mode & 3))
// TODO WGJA WIP: 			return -EBADF;
// TODO WGJA WIP: 		file_lock.fl_type = F_RDLCK;
// TODO WGJA WIP: 		break;
// TODO WGJA WIP: 	case F_EXLCK :
// TODO WGJA WIP: 		if (!(filp->f_mode & 3))
// TODO WGJA WIP: 			return -EBADF;
// TODO WGJA WIP: 		file_lock.fl_type = F_WRLCK;
// TODO WGJA WIP: 		break;
// TODO WGJA WIP: 	case F_UNLCK :
// TODO WGJA WIP: 		break;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP:   	/*
// TODO WGJA WIP:   	 * Scan for a conflicting lock ...
// TODO WGJA WIP:   	 */
  // TODO WGJA WIP: 
// TODO WGJA WIP: 	if (file_lock.fl_type != F_UNLCK) {
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 		for (fl = filp->f_inode->i_flock; fl != NULL; fl = fl->fl_next) {
// TODO WGJA WIP: 			if (!conflict(&file_lock, fl))
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			/*
// TODO WGJA WIP: 			 * File is locked by another process. If this is
// TODO WGJA WIP: 			 * F_SETLKW wait for the lock to be released.
// TODO WGJA WIP: 			 * FIXME: We need to check for deadlocks here.
// TODO WGJA WIP: 			 */
// TODO WGJA WIP: 			if (cmd == F_SETLKW) {
// TODO WGJA WIP: 				if (current->signal & ~current->blocked)
// TODO WGJA WIP: 					return -ERESTARTSYS;
// TODO WGJA WIP: 				interruptible_sleep_on(&fl->fl_wait);
// TODO WGJA WIP: 				if (current->signal & ~current->blocked)
// TODO WGJA WIP: 					return -ERESTARTSYS;
// TODO WGJA WIP: 				goto repeat;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			return -EAGAIN;
// TODO WGJA WIP:   		}
// TODO WGJA WIP:   	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Lock doesn't conflict with any other lock ...
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 
// TODO WGJA WIP: 	return lock_it(filp, &file_lock);
// TODO WGJA WIP: }

/*
 * This function is called when the file is closed.
 */

void fcntl_remove_locks(struct task_struct *task, struct file *filp)
{
	struct file_lock *fl;
	struct file_lock **before;

	/* Find first lock owned by caller ... */

	before = &filp->f_inode->i_flock;
	while ((fl = *before) && task != fl->fl_owner)
		before = &fl->fl_next;

	/* The list is sorted by owner ... */

	while ((fl = *before) && task == fl->fl_owner)
		free_lock(before);
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Verify a "struct flock" and copy it to a "struct file_lock" ...
// TODO WGJA WIP:  * Result is a boolean indicating success.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: static int copy_flock(struct file *filp, struct file_lock *fl, struct flock *l)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	off_t start;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!filp->f_inode)	/* just in case */
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (!S_ISREG(filp->f_inode->i_mode))
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (l->l_type != F_UNLCK && l->l_type != F_RDLCK && l->l_type != F_WRLCK
// TODO WGJA WIP: 	 && l->l_type != F_SHLCK && l->l_type != F_EXLCK)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	switch (l->l_whence) {
// TODO WGJA WIP: 	case 0 /*SEEK_SET*/ : start = 0; break;
// TODO WGJA WIP: 	case 1 /*SEEK_CUR*/ : start = filp->f_pos; break;
// TODO WGJA WIP: 	case 2 /*SEEK_END*/ : start = filp->f_inode->i_size; break;
// TODO WGJA WIP: 	default : return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if ((start += l->l_start) < 0 || l->l_len < 0)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	fl->fl_type = l->l_type;
// TODO WGJA WIP: 	fl->fl_start = start;	/* we record the absolute position */
// TODO WGJA WIP: 	fl->fl_whence = 0;	/* FIXME: do we record {l_start} as passed? */
// TODO WGJA WIP: 	if (l->l_len == 0 || (fl->fl_end = start + l->l_len - 1) < 0)
// TODO WGJA WIP: 		fl->fl_end = OFFSET_MAX;
// TODO WGJA WIP: 	fl->fl_owner = current;
// TODO WGJA WIP: 	fl->fl_wait = NULL;		/* just for cleanliness */
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Determine if lock {sys_fl} blocks lock {caller_fl} ...
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: static int conflict(struct file_lock *caller_fl, struct file_lock *sys_fl)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (caller_fl->fl_owner == sys_fl->fl_owner)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (!overlap(caller_fl, sys_fl))
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	switch (caller_fl->fl_type) {
// TODO WGJA WIP: 	case F_RDLCK :
// TODO WGJA WIP: 		return sys_fl->fl_type != F_RDLCK;
// TODO WGJA WIP: 	case F_WRLCK :
// TODO WGJA WIP: 		return 1;	/* overlapping region not owned by caller */
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;	/* shouldn't get here, but just in case */
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static int overlap(struct file_lock *fl1, struct file_lock *fl2)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	return fl1->fl_end >= fl2->fl_start && fl2->fl_end >= fl1->fl_start;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Add a lock to a file ...
// TODO WGJA WIP:  * Result is 0 for success or -ENOLCK.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * We merge adjacent locks whenever possible.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * WARNING: We assume the lock doesn't conflict with any other lock.
// TODO WGJA WIP:  */
  // TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Rewritten by Kai Petzke:
// TODO WGJA WIP:  * We sort the lock list first by owner, then by the starting address.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * To make freeing a lock much faster, we keep a pointer to the lock before the
// TODO WGJA WIP:  * actual one. But the real gain of the new coding was, that lock_it() and
// TODO WGJA WIP:  * unlock_it() became one function.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * To all purists: Yes, I use a few goto's. Just pass on to the next function.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: static int lock_it(struct file *filp, struct file_lock *caller)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct file_lock *fl;
// TODO WGJA WIP: 	struct file_lock *left = 0;
// TODO WGJA WIP: 	struct file_lock *right = 0;
// TODO WGJA WIP: 	struct file_lock **before;
// TODO WGJA WIP: 	int added = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Find the first old lock with the same owner as the new lock.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 
// TODO WGJA WIP: 	before = &filp->f_inode->i_flock;
// TODO WGJA WIP: 	while ((fl = *before) && caller->fl_owner != fl->fl_owner)
// TODO WGJA WIP: 		before = &fl->fl_next;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Look up all locks of this owner.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 
// TODO WGJA WIP: 	while ((fl = *before) && caller->fl_owner == fl->fl_owner) {
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * Detect adjacent or overlapping regions (if same lock type)
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		if (caller->fl_type == fl->fl_type) {
// TODO WGJA WIP: 			if (fl->fl_end < caller->fl_start - 1)
// TODO WGJA WIP: 				goto next_lock;
// TODO WGJA WIP: 			/*
// TODO WGJA WIP: 			 * If the next lock in the list has entirely bigger
// TODO WGJA WIP: 			 * addresses than the new one, insert the lock here.
// TODO WGJA WIP: 			 */
// TODO WGJA WIP: 			if (fl->fl_start > caller->fl_end + 1)
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 
// TODO WGJA WIP: 			/*
// TODO WGJA WIP: 			 * If we come here, the new and old lock are of the
// TODO WGJA WIP: 			 * same type and adjacent or overlapping. Make one
// TODO WGJA WIP: 			 * lock yielding from the lower start address of both
// TODO WGJA WIP: 			 * locks to the higher end address.
// TODO WGJA WIP: 			 */
// TODO WGJA WIP: 			if (fl->fl_start > caller->fl_start)
// TODO WGJA WIP: 				fl->fl_start = caller->fl_start;
// TODO WGJA WIP: 			else
// TODO WGJA WIP: 				caller->fl_start = fl->fl_start;
// TODO WGJA WIP: 			if (fl->fl_end < caller->fl_end)
// TODO WGJA WIP: 				fl->fl_end = caller->fl_end;
// TODO WGJA WIP: 			else
// TODO WGJA WIP: 				caller->fl_end = fl->fl_end;
// TODO WGJA WIP: 			if (added) {
// TODO WGJA WIP: 				free_lock(before);
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			caller = fl;
// TODO WGJA WIP: 			added = 1;
// TODO WGJA WIP: 			goto next_lock;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * Processing for different lock types is a bit more complex.
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		if (fl->fl_end < caller->fl_start)
// TODO WGJA WIP: 			goto next_lock;
// TODO WGJA WIP: 		if (fl->fl_start > caller->fl_end)
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		if (caller->fl_type == F_UNLCK)
// TODO WGJA WIP: 			added = 1;
// TODO WGJA WIP: 		if (fl->fl_start < caller->fl_start)
// TODO WGJA WIP: 			left = fl;
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * If the next lock in the list has a higher end address than
// TODO WGJA WIP: 		 * the new one, insert the new one here.
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: 		if (fl->fl_end > caller->fl_end) {
// TODO WGJA WIP: 			right = fl;
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (fl->fl_start >= caller->fl_start) {
// TODO WGJA WIP: 			/*
// TODO WGJA WIP: 			 * The new lock completely replaces an old one (This may
// TODO WGJA WIP: 			 * happen several times).
// TODO WGJA WIP: 			 */
// TODO WGJA WIP: 			if (added) {
// TODO WGJA WIP: 				free_lock(before);
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			/*
// TODO WGJA WIP: 			 * Replace the old lock with the new one. Wake up
// TODO WGJA WIP: 			 * anybody waiting for the old one, as the change in
// TODO WGJA WIP: 			 * lock type migth satisfy his needs.
// TODO WGJA WIP: 			 */
// TODO WGJA WIP: 			wake_up(&fl->fl_wait);
// TODO WGJA WIP: 			fl->fl_start = caller->fl_start;
// TODO WGJA WIP: 			fl->fl_end   = caller->fl_end;
// TODO WGJA WIP: 			fl->fl_type  = caller->fl_type;
// TODO WGJA WIP: 			fl->fl_wait  = 0;
// TODO WGJA WIP: 			caller = fl;
// TODO WGJA WIP: 			added = 1;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		/*
// TODO WGJA WIP: 		 * Go on to next lock.
// TODO WGJA WIP: 		 */
// TODO WGJA WIP: next_lock:
// TODO WGJA WIP: 		before = &(*before)->fl_next;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (! added) {
// TODO WGJA WIP: 		if (caller->fl_type == F_UNLCK)
// TODO WGJA WIP: 			return -EINVAL;
// TODO WGJA WIP: 		if (! (caller = alloc_lock(before, caller)))
// TODO WGJA WIP: 			return -ENOLCK;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (right) {
// TODO WGJA WIP: 		if (left == right) {
// TODO WGJA WIP: 			/*
// TODO WGJA WIP: 			 * The new lock breaks the old one in two pieces, so we
// TODO WGJA WIP: 			 * have to allocate one more lock (in this case, even
// TODO WGJA WIP: 			 * F_UNLCK may fail!).
// TODO WGJA WIP: 			 */
// TODO WGJA WIP: 			if (! (left = alloc_lock(before, right))) {
// TODO WGJA WIP: 				if (! added)
// TODO WGJA WIP: 					free_lock(before);
// TODO WGJA WIP: 				return -ENOLCK;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		right->fl_start = caller->fl_end + 1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (left)
// TODO WGJA WIP: 		left->fl_end = caller->fl_start - 1;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * File_lock() inserts a lock at the position pos of the linked list.
// TODO WGJA WIP:  */
// TODO WGJA WIP: 
// TODO WGJA WIP: static struct file_lock *alloc_lock(struct file_lock **pos,
// TODO WGJA WIP: 				    struct file_lock *fl)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct file_lock *tmp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	tmp = file_lock_free_list;
// TODO WGJA WIP: 	if (tmp == NULL)
// TODO WGJA WIP: 		return NULL;			/* no available entry */
// TODO WGJA WIP: 	if (tmp->fl_owner != NULL)
// TODO WGJA WIP: 		panic("alloc_lock: broken free list\n");
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* remove from free list */
// TODO WGJA WIP: 	file_lock_free_list = tmp->fl_next;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	*tmp = *fl;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	tmp->fl_next = *pos;	/* insert into file's list */
// TODO WGJA WIP: 	*pos = tmp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	tmp->fl_owner = current;	/* FIXME: needed? */
// TODO WGJA WIP: 	tmp->fl_wait = NULL;
// TODO WGJA WIP: 	return tmp;
// TODO WGJA WIP: }

/*
 * Add a lock to the free list ...
 */

static void free_lock(struct file_lock **fl_p)
{
	struct file_lock *fl;

	fl = *fl_p;
	if (fl->fl_owner == NULL)	/* sanity check */
		panic("free_lock: broken lock list\n");

	*fl_p = (*fl_p)->fl_next;

	fl->fl_next = file_lock_free_list;	/* add to free list */
	file_lock_free_list = fl;
	fl->fl_owner = NULL;			/* for sanity checks */

	wake_up(&fl->fl_wait);
}
