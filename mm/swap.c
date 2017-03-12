/*
 *  linux/mm/swap.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * This file should contain most things doing the swapping from/to disk.
 * Started 18.12.91
 */

#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/stat.h>

#include <asm/system.h> /* for cli()/sti() */
#include <asm/bitops.h>

#define MAX_SWAPFILES 8

#define SWP_USED	1
#define SWP_WRITEOK	3

#define SWP_TYPE(entry) (((entry) & 0xfe) >> 1)
#define SWP_OFFSET(entry) ((entry) >> PAGE_SHIFT)
#define SWP_ENTRY(type,offset) (((type) << 1) | ((offset) << PAGE_SHIFT))

static int nr_swapfiles = 0;
static struct wait_queue * lock_queue = (wait_queue *) NULL;

static struct swap_info_struct {
	unsigned long flags;
	struct inode * swap_file;
	unsigned int swap_device;
	unsigned char * swap_map;
	unsigned char * swap_lockmap;
	int pages;
	int lowest_bit;
	int highest_bit;
} swap_info[MAX_SWAPFILES];

extern unsigned long free_page_list;
extern int shm_swap (int);

/*
 * The following are used to make sure we don't thrash too much...
 * NOTE!! NR_LAST_FREE_PAGES must be a power of 2...
 */
#define NR_LAST_FREE_PAGES 32
static unsigned long last_free_pages[NR_LAST_FREE_PAGES] = {0,};

#define SWAP_BITS PAGE_SIZE

void rw_swap_page(int rw, unsigned long entry, char * buf)
{
	unsigned long type, offset;
	struct swap_info_struct * p;

	type = SWP_TYPE(entry);
	if (type >= nr_swapfiles) {
		printk("Internal error: bad swap-device\n");
		return;
	}
	p = &swap_info[type];
	offset = SWP_OFFSET(entry);
	if (offset >= SWAP_BITS) {
		printk("rw_swap_page: weirdness\n");
		return;
	}
	if (!(p->flags & SWP_USED)) {
		printk("Trying to swap to unused swap-device\n");
		return;
	}
	while (set_bit(offset,p->swap_lockmap))
		sleep_on(&lock_queue);
	if (p->swap_device) {
		ll_rw_page(rw,p->swap_device,offset,buf);
	} else if (p->swap_file) {
		unsigned int zones[8];
		unsigned int block;
		int i, j;

		block = offset << (12 - p->swap_file->i_sb->s_blocksize_bits);

		for (i=0, j=0; j< PAGE_SIZE ; i++, j +=p->swap_file->i_sb->s_blocksize)
			if (!(zones[i] = bmap(p->swap_file,block++))) {
				printk("rw_swap_page: bad swap file\n");
				return;
			}
		ll_rw_swap_file(rw,p->swap_file->i_dev, zones, i,buf);
	} else
		printk("re_swap_page: no swap file or device\n");
	if (!clear_bit(offset,p->swap_lockmap))
		printk("rw_swap_page: lock already cleared\n");
	wake_up(&lock_queue);
}

unsigned int get_swap_page(void)
{
	struct swap_info_struct * p;
	unsigned int offset, type;

	p = swap_info;
	for (type = 0 ; type < nr_swapfiles ; type++,p++) {
		if ((p->flags & SWP_WRITEOK) != SWP_WRITEOK)
			continue;
		for (offset = p->lowest_bit; offset <= p->highest_bit ; offset++) {
			if (p->swap_map[offset])
				continue;
			p->swap_map[offset] = 1;
			nr_swap_pages--;
			if (offset == p->highest_bit)
				p->highest_bit--;
			p->lowest_bit = offset;
			return SWP_ENTRY(type,offset);
		}
	}
	return 0;
}

unsigned long swap_duplicate(unsigned long entry)
{
	struct swap_info_struct * p;
	unsigned long offset, type;

	if (!entry)
		return 0;
	offset = SWP_OFFSET(entry);
	type = SWP_TYPE(entry);
	if (type == SHM_SWP_TYPE)
		return entry;
	if (type >= nr_swapfiles) {
		printk("Trying to duplicate nonexistent swap-page\n");
		return 0;
	}
	p = type + swap_info;
	if (offset >= SWAP_BITS) {
		printk("swap_free: weirness\n");
		return 0;
	}
	if (!p->swap_map[offset]) {
		printk("swap_duplicate: trying to duplicate unused page\n");
		return 0;
	}
	p->swap_map[offset]++;
	return entry;
}

void swap_free(unsigned long entry)
{
	struct swap_info_struct * p;
	unsigned long offset, type;

	if (!entry)
		return;
	type = SWP_TYPE(entry);
	if (type == SHM_SWP_TYPE)
		return;
	if (type >= nr_swapfiles) {
		printk("Trying to free nonexistent swap-page\n");
		return;
	}
	p = & swap_info[type];
	offset = SWP_OFFSET(entry);
	if (offset >= SWAP_BITS) {
		printk("swap_free: weirness\n");
		return;
	}
	if (!(p->flags & SWP_USED)) {
		printk("Trying to free swap from unused swap-device\n");
		return;
	}
	while (set_bit(offset,p->swap_lockmap))
		sleep_on(&lock_queue);
	if (offset < p->lowest_bit)
		p->lowest_bit = offset;
	if (offset > p->highest_bit)
		p->highest_bit = offset;
	if (!p->swap_map[offset])
		printk("swap_free: swap-space map bad (entry %08x)\n",entry);
	else
		if (!--p->swap_map[offset])
			nr_swap_pages++;
	if (!clear_bit(offset,p->swap_lockmap))
		printk("swap_free: lock already cleared\n");
	wake_up(&lock_queue);
}

void swap_in(unsigned long *table_ptr)
{
	unsigned long entry;
	unsigned long page;

	entry = *table_ptr;
	if (PAGE_PRESENT & entry) {
		printk("trying to swap in present page\n");
		return;
	}
	if (!entry) {
		printk("No swap page in swap_in\n");
		return;
	}
	if (SWP_TYPE(entry) == SHM_SWP_TYPE) {
		shm_no_page ((unsigned long *) table_ptr);
		return;
	}
	if (!(page = get_free_page(GFP_KERNEL))) {
		oom(current);
		page = BAD_PAGE;
	} else	
		read_swap_page(entry, (char *) page);
	if (*table_ptr != entry) {
		free_page(page);
		return;
	}
	*table_ptr = page | (PAGE_DIRTY | PAGE_PRIVATE);
	swap_free(entry);
}

static inline int try_to_swap_out(unsigned long * table_ptr)
{
	int i;
	unsigned long page;
	unsigned long entry;

	page = *table_ptr;
	if (!(PAGE_PRESENT & page))
		return 0;
	if (page >= high_memory)
		return 0;
	if (mem_map[MAP_NR(page)] & MAP_PAGE_RESERVED)
		return 0;
	if (PAGE_ACCESSED & page) {
		*table_ptr &= ~PAGE_ACCESSED;
		return 0;
	}
	for (i = 0; i < NR_LAST_FREE_PAGES; i++)
		if (last_free_pages[i] == (page & PAGE_MASK))
			return 0;
	if (PAGE_DIRTY & page) {
		page &= PAGE_MASK;
		if (mem_map[MAP_NR(page)] != 1)
			return 0;
		if (!(entry = get_swap_page()))
			return 0;
		*table_ptr = entry;
		invalidate();
		write_swap_page(entry, (char *) page);
		free_page(page);
		return 1;
	}
	page &= PAGE_MASK;
	*table_ptr = 0;
	invalidate();
	free_page(page);
	return 1 + mem_map[MAP_NR(page)];
}

/*
 * sys_idle() does nothing much: it just searches for likely candidates for
 * swapping out or forgetting about. This speeds up the search when we
 * actually have to swap.
 */
extern "C" int sys_idle(void)
{
	__asm__ ("hlt" :::);
	need_resched = 1;
	return 0;
}

/*
 * A new implementation of swap_out().  We do not swap complete processes,
 * but only a small number of blocks, before we continue with the next
 * process.  The number of blocks actually swapped is determined on the
 * number of page faults, that this process actually had in the last time,
 * so we won't swap heavily used processes all the time ...
 *
 * Note: the priority argument is a hint on much CPU to waste with the
 *       swap block search, not a hint, of how much blocks to swap with
 *       each process.
 *
 * (C) 1993 Kai Petzke, wpp@marie.physik.tu-berlin.de
 */
#ifdef NEW_SWAP
/*
 * These are the miminum and maximum number of pages to swap from one process,
 * before proceeding to the next:
 */
#define SWAP_MIN	4
#define SWAP_MAX	32

/*
 * The actual number of pages to swap is determined as:
 * SWAP_RATIO / (number of recent major page faults)
 */
#define SWAP_RATIO	128

static int swap_out(unsigned int priority)
{
    static int swap_task;
    int table;
    int page;
    long pg_table;
    int loop;
    int counter = NR_TASKS * 2 >> priority;
    struct task_struct *p;

    counter = NR_TASKS * 2 >> priority;
    for(; counter >= 0; counter--, swap_task++) {
	/*
	 * Check that swap_task is suitable for swapping.  If not, look for
	 * the next suitable process.
	 */
	loop = 0;
	while(1) {
	    if(swap_task >= NR_TASKS) {
		swap_task = 1;
		if(loop)
		    /* all processes are unswappable or already swapped out */
		    return 0;
		loop = 1;
	    }

	    p = task[swap_task];
	    if(p && p->swappable && p->rss)
		break;

	    swap_task++;
	}

	/*
	 * Determine the number of pages to swap from this process.
	 */
	if(! p -> swap_cnt) {
	    p->dec_flt = (p->dec_flt * 3) / 4 + p->maj_flt - p->old_maj_flt;
	    p->old_maj_flt = p->maj_flt;

	    if(p->dec_flt >= SWAP_RATIO / SWAP_MIN) {
		p->dec_flt = SWAP_RATIO / SWAP_MIN;
		p->swap_cnt = SWAP_MIN;
	    } else if(p->dec_flt <= SWAP_RATIO / SWAP_MAX)
		p->swap_cnt = SWAP_MAX;
	    else
		p->swap_cnt = SWAP_RATIO / p->dec_flt;
	}

	/*
	 * Go through process' page directory.
	 */
	for(table = p->swap_table; table < 1024; table++) {
	    pg_table = ((unsigned long *) p->tss.cr3)[table];
	    if(pg_table >= high_memory)
		    continue;
	    if(mem_map[MAP_NR(pg_table)] & MAP_PAGE_RESERVED)
		    continue;
	    if(!(PAGE_PRESENT & pg_table)) {
		    printk("swap_out: bad page-table at pg_dir[%d]: %08x\n",
			    table, pg_table);
		    ((unsigned long *) p->tss.cr3)[table] = 0;
		    continue;
	    }
	    pg_table &= 0xfffff000;

	    /*
	     * Go through this page table.
	     */
	    for(page = p->swap_page; page < 1024; page++) {
		switch(try_to_swap_out(page + (unsigned long *) pg_table)) {
		    case 0:
			break;

		    case 1:
			p->rss--;
			/* continue with the following page the next time */
			p->swap_table = table;
			p->swap_page  = page + 1;
			if((--p->swap_cnt) == 0)
			    swap_task++;
			return 1;

		    default:
			p->rss--;
			break;
		}
	    }

	    p->swap_page = 0;
	}

	/*
	 * Finish work with this process, if we reached the end of the page
	 * directory.  Mark restart from the beginning the next time.
	 */
	p->swap_table = 0;
    }
    return 0;
}

#else /* old swapping procedure */

/*
 * Go through the page tables, searching for a user page that
 * we can swap out.
 * 
 * We now check that the process is swappable (normally only 'init'
 * is un-swappable), allowing high-priority processes which cannot be
 * swapped out (things like user-level device drivers (Not implemented)).
 */
static int swap_out(unsigned int priority)
{
	static int swap_task = 1;
	static int swap_table = 0;
	static int swap_page = 0;
	int counter = NR_TASKS*8;
	int pg_table;
	struct task_struct * p;

	counter >>= priority;
check_task:
	if (counter-- < 0)
		return 0;
	if (swap_task >= NR_TASKS) {
		swap_task = 1;
		goto check_task;
	}
	p = task[swap_task];
	if (!p || !p->swappable) {
		swap_task++;
		goto check_task;
	}
check_dir:
	if (swap_table >= PTRS_PER_PAGE) {
		swap_table = 0;
		swap_task++;
		goto check_task;
	}
	pg_table = ((unsigned long *) p->tss.cr3)[swap_table];
	if (pg_table >= high_memory || (mem_map[MAP_NR(pg_table)] & MAP_PAGE_RESERVED)) {
		swap_table++;
		goto check_dir;
	}
	if (!(PAGE_PRESENT & pg_table)) {
		printk("bad page-table at pg_dir[%d]: %08x\n",
			swap_table,pg_table);
		((unsigned long *) p->tss.cr3)[swap_table] = 0;
		swap_table++;
		goto check_dir;
	}
	pg_table &= PAGE_MASK;
check_table:
	if (swap_page >= PTRS_PER_PAGE) {
		swap_page = 0;
		swap_table++;
		goto check_dir;
	}
	switch (try_to_swap_out(swap_page + (unsigned long *) pg_table)) {
		case 0: break;
		case 1: p->rss--; return 1;
		default: p->rss--;
	}
	swap_page++;
	goto check_table;
}

#endif

static int try_to_free_page(void)
{
	int i=6;

	while (i--) {
		if (shrink_buffers(i))
			return 1;
		if (shm_swap(i))
			return 1;
		if (swap_out(i))
			return 1;
	}
	return 0;
}

/*
 * Note that this must be atomic, or bad things will happen when
 * pages are requested in interrupts (as malloc can do). Thus the
 * cli/sti's.
 */
static inline void add_mem_queue(unsigned long addr, unsigned long * queue)
{
	addr &= PAGE_MASK;
	*(unsigned long *) addr = *queue;
	*queue = addr;
}

/*
 * Free_page() adds the page to the free lists. This is optimized for
 * fast normal cases (no error jumps taken normally).
 *
 * The way to optimize jumps for gcc-2.2.2 is to:
 *  - select the "normal" case and put it inside the if () { XXX }
 *  - no else-statements if you can avoid them
 *
 * With the above two rules, you get a straight-line execution path
 * for the normal case, giving better asm-code.
 */
void free_page(unsigned long addr)
{
	if (addr < high_memory) {
		unsigned short * map = mem_map + MAP_NR(addr);

		if (*map) {
			if (!(*map & MAP_PAGE_RESERVED)) {
				unsigned long flag;

				save_flags(flag);
				cli();
				if (!--*map) {
					if (nr_secondary_pages < MAX_SECONDARY_PAGES) {
						add_mem_queue(addr,&secondary_page_list);
						nr_secondary_pages++;
						restore_flags(flag);
						return;
					}
					add_mem_queue(addr,&free_page_list);
					nr_free_pages++;
				}
				restore_flags(flag);
			}
			return;
		}
		printk("Trying to free free memory (%08x): memory probabably corrupted\n",addr);
		printk("PC = %08x\n",*(((unsigned long *)&addr)-1));
		return;
	}
}

/*
 * This is one ugly macro, but it simplifies checking, and makes
 * this speed-critical place reasonably fast, especially as we have
 * to do things with the interrupt flag etc.
 *
 * Note that this #define is heavily optimized to give fast code
 * for the normal case - the if-statements are ordered so that gcc-2.2.2
 * will make *no* jumps for the normal code. Don't touch unless you
 * know what you are doing.
 */
#define REMOVE_FROM_MEM_QUEUE(queue,nr) \
	cli(); \
	if ((result = queue) != 0) { \
		if (!(result & ~PAGE_MASK) && result < high_memory) { \
			queue = *(unsigned long *) result; \
			if (!mem_map[MAP_NR(result)]) { \
				mem_map[MAP_NR(result)] = 1; \
				nr--; \
last_free_pages[index = (index + 1) & (NR_LAST_FREE_PAGES - 1)] = result; \
				restore_flags(flag); \
				return result; \
			} \
			printk("Free page %08x has mem_map = %d\n", \
				result,mem_map[MAP_NR(result)]); \
		} else \
			printk("Result = 0x%08x - memory map destroyed\n", result); \
		queue = 0; \
		nr = 0; \
	} else if (nr) { \
		printk(#nr " is %d, but " #queue " is empty\n",nr); \
		nr = 0; \
	} \
	restore_flags(flag)

/*
 * Get physical address of first (actually last :-) free page, and mark it
 * used. If no free pages left, return 0.
 *
 * Note that this is one of the most heavily called functions in the kernel,
 * so it's a bit timing-critical (especially as we have to disable interrupts
 * in it). See the above macro which does most of the work, and which is
 * optimized for a fast normal path of execution.
 */
unsigned long __get_free_page(int priority)
{
	unsigned long result, flag;
	static unsigned long index = 0;

	/* this routine can be called at interrupt time via
	   malloc.  We want to make sure that the critical
	   sections of code have interrupts disabled. -RAB
	   Is this code reentrant? */

	save_flags(flag);
repeat:
	REMOVE_FROM_MEM_QUEUE(free_page_list,nr_free_pages);
	if (priority == GFP_BUFFER)
		return 0;
	if (priority != GFP_ATOMIC)
		if (try_to_free_page())
			goto repeat;
	REMOVE_FROM_MEM_QUEUE(secondary_page_list,nr_secondary_pages);
	return 0;
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Trying to stop swapping from a file is fraught with races, so
// TODO WGJA WIP:  * we repeat quite a bit here when we have to pause. swapoff()
// TODO WGJA WIP:  * isn't exactly timing-critical, so who cares?
// TODO WGJA WIP:  */
// TODO WGJA WIP: static int try_to_unuse(unsigned int type)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int nr, pgt, pg;
// TODO WGJA WIP: 	unsigned long page, *ppage;
// TODO WGJA WIP: 	unsigned long tmp = 0;
// TODO WGJA WIP: 	struct task_struct *p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	nr = 0;
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * When we have to sleep, we restart the whole algorithm from the same
// TODO WGJA WIP:  * task we stopped in. That at least rids us of all races.
// TODO WGJA WIP:  */
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 	for (; nr < NR_TASKS ; nr++) {
// TODO WGJA WIP: 		p = task[nr];
// TODO WGJA WIP: 		if (!p)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		for (pgt = 0 ; pgt < PTRS_PER_PAGE ; pgt++) {
// TODO WGJA WIP: 			ppage = pgt + ((unsigned long *) p->tss.cr3);
// TODO WGJA WIP: 			page = *ppage;
// TODO WGJA WIP: 			if (!page)
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			if (!(page & PAGE_PRESENT) || (page >= high_memory))
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			if (mem_map[MAP_NR(page)] & MAP_PAGE_RESERVED)
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			ppage = (unsigned long *) (page & PAGE_MASK);	
// TODO WGJA WIP: 			for (pg = 0 ; pg < PTRS_PER_PAGE ; pg++,ppage++) {
// TODO WGJA WIP: 				page = *ppage;
// TODO WGJA WIP: 				if (!page)
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				if (page & PAGE_PRESENT)
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				if (SWP_TYPE(page) != type)
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				if (!tmp) {
// TODO WGJA WIP: 					if (!(tmp = __get_free_page(GFP_KERNEL)))
// TODO WGJA WIP: 						return -ENOMEM;
// TODO WGJA WIP: 					goto repeat;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				read_swap_page(page, (char *) tmp);
// TODO WGJA WIP: 				if (*ppage == page) {
// TODO WGJA WIP: 					*ppage = tmp | (PAGE_DIRTY | PAGE_PRIVATE);
// TODO WGJA WIP: 					++p->rss;
// TODO WGJA WIP: 					swap_free(page);
// TODO WGJA WIP: 					tmp = 0;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				goto repeat;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	free_page(tmp);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: extern "C" int sys_swapoff(const char * specialfile)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct swap_info_struct * p;
// TODO WGJA WIP: 	struct inode * inode;
// TODO WGJA WIP: 	unsigned int type;
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	i = namei(specialfile,&inode);
// TODO WGJA WIP: 	if (i)
// TODO WGJA WIP: 		return i;
// TODO WGJA WIP: 	p = swap_info;
// TODO WGJA WIP: 	for (type = 0 ; type < nr_swapfiles ; type++,p++) {
// TODO WGJA WIP: 		if ((p->flags & SWP_WRITEOK) != SWP_WRITEOK)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (p->swap_file) {
// TODO WGJA WIP: 			if (p->swap_file == inode)
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 		} else {
// TODO WGJA WIP: 			if (!S_ISBLK(inode->i_mode))
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			if (p->swap_device == inode->i_rdev)
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	iput(inode);
// TODO WGJA WIP: 	if (type >= nr_swapfiles)
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	p->flags = SWP_USED;
// TODO WGJA WIP: 	i = try_to_unuse(type);
// TODO WGJA WIP: 	if (i) {
// TODO WGJA WIP: 		p->flags = SWP_WRITEOK;
// TODO WGJA WIP: 		return i;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	nr_swap_pages -= p->pages;
// TODO WGJA WIP: 	iput(p->swap_file);
// TODO WGJA WIP: 	p->swap_file = NULL;
// TODO WGJA WIP: 	p->swap_device = 0;
// TODO WGJA WIP: 	free_page((long) p->swap_map);
// TODO WGJA WIP: 	p->swap_map = NULL;
// TODO WGJA WIP: 	free_page((long) p->swap_lockmap);
// TODO WGJA WIP: 	p->swap_lockmap = NULL;
// TODO WGJA WIP: 	p->flags = 0;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Written 01/25/92 by Simmule Turner, heavily changed by Linus.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * The swapon system call
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" int sys_swapon(const char * specialfile)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct swap_info_struct * p;
// TODO WGJA WIP: 	struct inode * swap_inode;
// TODO WGJA WIP: 	unsigned int type;
// TODO WGJA WIP: 	unsigned char * tmp;
// TODO WGJA WIP: 	int i,j;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!suser())
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	p = swap_info;
// TODO WGJA WIP: 	for (type = 0 ; type < nr_swapfiles ; type++,p++)
// TODO WGJA WIP: 		if (!(p->flags & SWP_USED))
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 	if (type >= MAX_SWAPFILES)
// TODO WGJA WIP: 		return -EPERM;
// TODO WGJA WIP: 	if (type >= nr_swapfiles)
// TODO WGJA WIP: 		nr_swapfiles = type+1;
// TODO WGJA WIP: 	p->flags = SWP_USED;
// TODO WGJA WIP: 	p->swap_file = NULL;
// TODO WGJA WIP: 	p->swap_device = 0;
// TODO WGJA WIP: 	p->swap_map = NULL;
// TODO WGJA WIP: 	p->swap_lockmap = NULL;
// TODO WGJA WIP: 	p->lowest_bit = 0;
// TODO WGJA WIP: 	p->highest_bit = 0;
// TODO WGJA WIP: 	i = namei(specialfile,&swap_inode);
// TODO WGJA WIP: 	if (i) {
// TODO WGJA WIP: 		p->flags = 0;
// TODO WGJA WIP: 		return i;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (swap_inode->i_count != 1) {
// TODO WGJA WIP: 		iput(swap_inode);
// TODO WGJA WIP: 		p->flags = 0;
// TODO WGJA WIP: 		return -EBUSY;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (S_ISBLK(swap_inode->i_mode)) {
// TODO WGJA WIP: 		p->swap_device = swap_inode->i_rdev;
// TODO WGJA WIP: 		iput(swap_inode);
// TODO WGJA WIP: 		if (!p->swap_device) {
// TODO WGJA WIP: 			p->flags = 0;
// TODO WGJA WIP: 			return -ENODEV;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		for (i = 0 ; i < nr_swapfiles ; i++) {
// TODO WGJA WIP: 			if (i == type)
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			if (p->swap_device == swap_info[i].swap_device) {
// TODO WGJA WIP: 				p->swap_device = 0;
// TODO WGJA WIP: 				p->flags = 0;
// TODO WGJA WIP: 				return -EBUSY;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	} else if (S_ISREG(swap_inode->i_mode))
// TODO WGJA WIP: 		p->swap_file = swap_inode;
// TODO WGJA WIP: 	else {
// TODO WGJA WIP: 		iput(swap_inode);
// TODO WGJA WIP: 		p->flags = 0;
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	tmp = (unsigned char *) get_free_page(GFP_USER);
// TODO WGJA WIP: 	p->swap_lockmap = (unsigned char *) get_free_page(GFP_USER);
// TODO WGJA WIP: 	if (!tmp || !p->swap_lockmap) {
// TODO WGJA WIP: 		printk("Unable to start swapping: out of memory :-)\n");
// TODO WGJA WIP: 		free_page((long) tmp);
// TODO WGJA WIP: 		free_page((long) p->swap_lockmap);
// TODO WGJA WIP: 		iput(p->swap_file);
// TODO WGJA WIP: 		p->swap_device = 0;
// TODO WGJA WIP: 		p->swap_file = NULL;
// TODO WGJA WIP: 		p->swap_map = NULL;
// TODO WGJA WIP: 		p->swap_lockmap = NULL;
// TODO WGJA WIP: 		p->flags = 0;
// TODO WGJA WIP: 		return -ENOMEM;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	read_swap_page(SWP_ENTRY(type,0), (char *) tmp);
// TODO WGJA WIP: 	if (memcmp("SWAP-SPACE",tmp+4086,10)) {
// TODO WGJA WIP: 		printk("Unable to find swap-space signature\n");
// TODO WGJA WIP: 		free_page((long) tmp);
// TODO WGJA WIP: 		free_page((long) p->swap_lockmap);
// TODO WGJA WIP: 		iput(p->swap_file);
// TODO WGJA WIP: 		p->swap_device = 0;
// TODO WGJA WIP: 		p->swap_file = NULL;
// TODO WGJA WIP: 		p->swap_map = NULL;
// TODO WGJA WIP: 		p->swap_lockmap = NULL;
// TODO WGJA WIP: 		p->flags = 0;
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	memset(tmp+PAGE_SIZE-10,0,10);
// TODO WGJA WIP: 	j = 0;
// TODO WGJA WIP: 	p->lowest_bit = 0;
// TODO WGJA WIP: 	p->highest_bit = 0;
// TODO WGJA WIP: 	for (i = 1 ; i < SWAP_BITS ; i++)
// TODO WGJA WIP: 		if (test_bit(i,tmp)) {
// TODO WGJA WIP: 			if (!p->lowest_bit)
// TODO WGJA WIP: 				p->lowest_bit = i;
// TODO WGJA WIP: 			p->highest_bit = i;
// TODO WGJA WIP: 			j++;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	if (!j) {
// TODO WGJA WIP: 		printk("Empty swap-file\n");
// TODO WGJA WIP: 		free_page((long) tmp);
// TODO WGJA WIP: 		free_page((long) p->swap_lockmap);
// TODO WGJA WIP: 		iput(p->swap_file);
// TODO WGJA WIP: 		p->swap_device = 0;
// TODO WGJA WIP: 		p->swap_file = NULL;
// TODO WGJA WIP: 		p->swap_map = NULL;
// TODO WGJA WIP: 		p->swap_lockmap = NULL;
// TODO WGJA WIP: 		p->flags = 0;
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	i = SWAP_BITS;
// TODO WGJA WIP: 	while (i--)
// TODO WGJA WIP: 		if (test_bit(i,tmp))
// TODO WGJA WIP: 			tmp[i] = 0;
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			tmp[i] = 0x80;
// TODO WGJA WIP: 	tmp[0] = 0x80;
// TODO WGJA WIP: 	p->swap_map = tmp;
// TODO WGJA WIP: 	p->flags = SWP_WRITEOK;
// TODO WGJA WIP: 	p->pages = j;
// TODO WGJA WIP: 	nr_swap_pages += j;
// TODO WGJA WIP: 	printk("Adding Swap: %dk swap-space\n",j<<2);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void si_swapinfo(struct sysinfo *val)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned int i, j;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	val->freeswap = val->totalswap = 0;
// TODO WGJA WIP: 	for (i = 0; i < nr_swapfiles; i++) {
// TODO WGJA WIP: 		if (!(swap_info[i].flags & SWP_USED))
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		for (j = 0; j < SWAP_BITS; ++j)
// TODO WGJA WIP: 			switch (swap_info[i].swap_map[j]) {
// TODO WGJA WIP: 				case 128:
// TODO WGJA WIP: 					continue;
// TODO WGJA WIP: 				case 0:
// TODO WGJA WIP: 					++val->freeswap;
// TODO WGJA WIP: 				default:
// TODO WGJA WIP: 					++val->totalswap;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	val->freeswap <<= PAGE_SHIFT;
// TODO WGJA WIP: 	val->totalswap <<= PAGE_SHIFT;
// TODO WGJA WIP: 	return;
// TODO WGJA WIP: }
