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
// TODO WGJA WIP: #include <linux/string.h>
// TODO WGJA WIP: #include <linux/stat.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: #include <asm/system.h> /* for cli()/sti() */
// TODO WGJA WIP: #include <asm/bitops.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: #define MAX_SWAPFILES 8
// TODO WGJA WIP: 
// TODO WGJA WIP: #define SWP_USED	1
// TODO WGJA WIP: #define SWP_WRITEOK	3
// TODO WGJA WIP: 
// TODO WGJA WIP: #define SWP_TYPE(entry) (((entry) & 0xfe) >> 1)
// TODO WGJA WIP: #define SWP_OFFSET(entry) ((entry) >> PAGE_SHIFT)
// TODO WGJA WIP: #define SWP_ENTRY(type,offset) (((type) << 1) | ((offset) << PAGE_SHIFT))
// TODO WGJA WIP: 
// TODO WGJA WIP: static int nr_swapfiles = 0;
// TODO WGJA WIP: static struct wait_queue * lock_queue = NULL;
// TODO WGJA WIP: 
// TODO WGJA WIP: static struct swap_info_struct {
// TODO WGJA WIP: 	unsigned long flags;
// TODO WGJA WIP: 	struct inode * swap_file;
// TODO WGJA WIP: 	unsigned int swap_device;
// TODO WGJA WIP: 	unsigned char * swap_map;
// TODO WGJA WIP: 	unsigned char * swap_lockmap;
// TODO WGJA WIP: 	int pages;
// TODO WGJA WIP: 	int lowest_bit;
// TODO WGJA WIP: 	int highest_bit;
// TODO WGJA WIP: } swap_info[MAX_SWAPFILES];
// TODO WGJA WIP: 
// TODO WGJA WIP: extern unsigned long free_page_list;
// TODO WGJA WIP: extern int shm_swap (int);
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * The following are used to make sure we don't thrash too much...
// TODO WGJA WIP:  * NOTE!! NR_LAST_FREE_PAGES must be a power of 2...
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define NR_LAST_FREE_PAGES 32
// TODO WGJA WIP: static unsigned long last_free_pages[NR_LAST_FREE_PAGES] = {0,};
// TODO WGJA WIP: 
// TODO WGJA WIP: #define SWAP_BITS PAGE_SIZE
// TODO WGJA WIP: 
// TODO WGJA WIP: void rw_swap_page(int rw, unsigned long entry, char * buf)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long type, offset;
// TODO WGJA WIP: 	struct swap_info_struct * p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	type = SWP_TYPE(entry);
// TODO WGJA WIP: 	if (type >= nr_swapfiles) {
// TODO WGJA WIP: 		printk("Internal error: bad swap-device\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	p = &swap_info[type];
// TODO WGJA WIP: 	offset = SWP_OFFSET(entry);
// TODO WGJA WIP: 	if (offset >= SWAP_BITS) {
// TODO WGJA WIP: 		printk("rw_swap_page: weirdness\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!(p->flags & SWP_USED)) {
// TODO WGJA WIP: 		printk("Trying to swap to unused swap-device\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	while (set_bit(offset,p->swap_lockmap))
// TODO WGJA WIP: 		sleep_on(&lock_queue);
// TODO WGJA WIP: 	if (p->swap_device) {
// TODO WGJA WIP: 		ll_rw_page(rw,p->swap_device,offset,buf);
// TODO WGJA WIP: 	} else if (p->swap_file) {
// TODO WGJA WIP: 		unsigned int zones[8];
// TODO WGJA WIP: 		unsigned int block;
// TODO WGJA WIP: 		int i, j;
// TODO WGJA WIP: 
// TODO WGJA WIP: 		block = offset << (12 - p->swap_file->i_sb->s_blocksize_bits);
// TODO WGJA WIP: 
// TODO WGJA WIP: 		for (i=0, j=0; j< PAGE_SIZE ; i++, j +=p->swap_file->i_sb->s_blocksize)
// TODO WGJA WIP: 			if (!(zones[i] = bmap(p->swap_file,block++))) {
// TODO WGJA WIP: 				printk("rw_swap_page: bad swap file\n");
// TODO WGJA WIP: 				return;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		ll_rw_swap_file(rw,p->swap_file->i_dev, zones, i,buf);
// TODO WGJA WIP: 	} else
// TODO WGJA WIP: 		printk("re_swap_page: no swap file or device\n");
// TODO WGJA WIP: 	if (!clear_bit(offset,p->swap_lockmap))
// TODO WGJA WIP: 		printk("rw_swap_page: lock already cleared\n");
// TODO WGJA WIP: 	wake_up(&lock_queue);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: unsigned int get_swap_page(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct swap_info_struct * p;
// TODO WGJA WIP: 	unsigned int offset, type;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	p = swap_info;
// TODO WGJA WIP: 	for (type = 0 ; type < nr_swapfiles ; type++,p++) {
// TODO WGJA WIP: 		if ((p->flags & SWP_WRITEOK) != SWP_WRITEOK)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		for (offset = p->lowest_bit; offset <= p->highest_bit ; offset++) {
// TODO WGJA WIP: 			if (p->swap_map[offset])
// TODO WGJA WIP: 				continue;
// TODO WGJA WIP: 			p->swap_map[offset] = 1;
// TODO WGJA WIP: 			nr_swap_pages--;
// TODO WGJA WIP: 			if (offset == p->highest_bit)
// TODO WGJA WIP: 				p->highest_bit--;
// TODO WGJA WIP: 			p->lowest_bit = offset;
// TODO WGJA WIP: 			return SWP_ENTRY(type,offset);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: unsigned long swap_duplicate(unsigned long entry)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct swap_info_struct * p;
// TODO WGJA WIP: 	unsigned long offset, type;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!entry)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	offset = SWP_OFFSET(entry);
// TODO WGJA WIP: 	type = SWP_TYPE(entry);
// TODO WGJA WIP: 	if (type == SHM_SWP_TYPE)
// TODO WGJA WIP: 		return entry;
// TODO WGJA WIP: 	if (type >= nr_swapfiles) {
// TODO WGJA WIP: 		printk("Trying to duplicate nonexistent swap-page\n");
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	p = type + swap_info;
// TODO WGJA WIP: 	if (offset >= SWAP_BITS) {
// TODO WGJA WIP: 		printk("swap_free: weirness\n");
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!p->swap_map[offset]) {
// TODO WGJA WIP: 		printk("swap_duplicate: trying to duplicate unused page\n");
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	p->swap_map[offset]++;
// TODO WGJA WIP: 	return entry;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void swap_free(unsigned long entry)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct swap_info_struct * p;
// TODO WGJA WIP: 	unsigned long offset, type;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!entry)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	type = SWP_TYPE(entry);
// TODO WGJA WIP: 	if (type == SHM_SWP_TYPE)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (type >= nr_swapfiles) {
// TODO WGJA WIP: 		printk("Trying to free nonexistent swap-page\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	p = & swap_info[type];
// TODO WGJA WIP: 	offset = SWP_OFFSET(entry);
// TODO WGJA WIP: 	if (offset >= SWAP_BITS) {
// TODO WGJA WIP: 		printk("swap_free: weirness\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!(p->flags & SWP_USED)) {
// TODO WGJA WIP: 		printk("Trying to free swap from unused swap-device\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	while (set_bit(offset,p->swap_lockmap))
// TODO WGJA WIP: 		sleep_on(&lock_queue);
// TODO WGJA WIP: 	if (offset < p->lowest_bit)
// TODO WGJA WIP: 		p->lowest_bit = offset;
// TODO WGJA WIP: 	if (offset > p->highest_bit)
// TODO WGJA WIP: 		p->highest_bit = offset;
// TODO WGJA WIP: 	if (!p->swap_map[offset])
// TODO WGJA WIP: 		printk("swap_free: swap-space map bad (entry %08x)\n",entry);
// TODO WGJA WIP: 	else
// TODO WGJA WIP: 		if (!--p->swap_map[offset])
// TODO WGJA WIP: 			nr_swap_pages++;
// TODO WGJA WIP: 	if (!clear_bit(offset,p->swap_lockmap))
// TODO WGJA WIP: 		printk("swap_free: lock already cleared\n");
// TODO WGJA WIP: 	wake_up(&lock_queue);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void swap_in(unsigned long *table_ptr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long entry;
// TODO WGJA WIP: 	unsigned long page;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	entry = *table_ptr;
// TODO WGJA WIP: 	if (PAGE_PRESENT & entry) {
// TODO WGJA WIP: 		printk("trying to swap in present page\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!entry) {
// TODO WGJA WIP: 		printk("No swap page in swap_in\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (SWP_TYPE(entry) == SHM_SWP_TYPE) {
// TODO WGJA WIP: 		shm_no_page ((unsigned long *) table_ptr);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!(page = get_free_page(GFP_KERNEL))) {
// TODO WGJA WIP: 		oom(current);
// TODO WGJA WIP: 		page = BAD_PAGE;
// TODO WGJA WIP: 	} else	
// TODO WGJA WIP: 		read_swap_page(entry, (char *) page);
// TODO WGJA WIP: 	if (*table_ptr != entry) {
// TODO WGJA WIP: 		free_page(page);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	*table_ptr = page | (PAGE_DIRTY | PAGE_PRIVATE);
// TODO WGJA WIP: 	swap_free(entry);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: static inline int try_to_swap_out(unsigned long * table_ptr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 	unsigned long page;
// TODO WGJA WIP: 	unsigned long entry;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	page = *table_ptr;
// TODO WGJA WIP: 	if (!(PAGE_PRESENT & page))
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (page >= high_memory)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (mem_map[MAP_NR(page)] & MAP_PAGE_RESERVED)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (PAGE_ACCESSED & page) {
// TODO WGJA WIP: 		*table_ptr &= ~PAGE_ACCESSED;
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	for (i = 0; i < NR_LAST_FREE_PAGES; i++)
// TODO WGJA WIP: 		if (last_free_pages[i] == (page & PAGE_MASK))
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 	if (PAGE_DIRTY & page) {
// TODO WGJA WIP: 		page &= PAGE_MASK;
// TODO WGJA WIP: 		if (mem_map[MAP_NR(page)] != 1)
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		if (!(entry = get_swap_page()))
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		*table_ptr = entry;
// TODO WGJA WIP: 		invalidate();
// TODO WGJA WIP: 		write_swap_page(entry, (char *) page);
// TODO WGJA WIP: 		free_page(page);
// TODO WGJA WIP: 		return 1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	page &= PAGE_MASK;
// TODO WGJA WIP: 	*table_ptr = 0;
// TODO WGJA WIP: 	invalidate();
// TODO WGJA WIP: 	free_page(page);
// TODO WGJA WIP: 	return 1 + mem_map[MAP_NR(page)];
// TODO WGJA WIP: }

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

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * A new implementation of swap_out().  We do not swap complete processes,
// TODO WGJA WIP:  * but only a small number of blocks, before we continue with the next
// TODO WGJA WIP:  * process.  The number of blocks actually swapped is determined on the
// TODO WGJA WIP:  * number of page faults, that this process actually had in the last time,
// TODO WGJA WIP:  * so we won't swap heavily used processes all the time ...
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Note: the priority argument is a hint on much CPU to waste with the
// TODO WGJA WIP:  *       swap block search, not a hint, of how much blocks to swap with
// TODO WGJA WIP:  *       each process.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * (C) 1993 Kai Petzke, wpp@marie.physik.tu-berlin.de
// TODO WGJA WIP:  */
// TODO WGJA WIP: #ifdef NEW_SWAP
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * These are the miminum and maximum number of pages to swap from one process,
// TODO WGJA WIP:  * before proceeding to the next:
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define SWAP_MIN	4
// TODO WGJA WIP: #define SWAP_MAX	32
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * The actual number of pages to swap is determined as:
// TODO WGJA WIP:  * SWAP_RATIO / (number of recent major page faults)
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define SWAP_RATIO	128
// TODO WGJA WIP: 
// TODO WGJA WIP: static int swap_out(unsigned int priority)
// TODO WGJA WIP: {
// TODO WGJA WIP:     static int swap_task;
// TODO WGJA WIP:     int table;
// TODO WGJA WIP:     int page;
// TODO WGJA WIP:     long pg_table;
// TODO WGJA WIP:     int loop;
// TODO WGJA WIP:     int counter = NR_TASKS * 2 >> priority;
// TODO WGJA WIP:     struct task_struct *p;
// TODO WGJA WIP: 
// TODO WGJA WIP:     counter = NR_TASKS * 2 >> priority;
// TODO WGJA WIP:     for(; counter >= 0; counter--, swap_task++) {
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Check that swap_task is suitable for swapping.  If not, look for
// TODO WGJA WIP: 	 * the next suitable process.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	loop = 0;
// TODO WGJA WIP: 	while(1) {
// TODO WGJA WIP: 	    if(swap_task >= NR_TASKS) {
// TODO WGJA WIP: 		swap_task = 1;
// TODO WGJA WIP: 		if(loop)
// TODO WGJA WIP: 		    /* all processes are unswappable or already swapped out */
// TODO WGJA WIP: 		    return 0;
// TODO WGJA WIP: 		loop = 1;
// TODO WGJA WIP: 	    }
// TODO WGJA WIP: 
// TODO WGJA WIP: 	    p = task[swap_task];
// TODO WGJA WIP: 	    if(p && p->swappable && p->rss)
// TODO WGJA WIP: 		break;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	    swap_task++;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Determine the number of pages to swap from this process.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	if(! p -> swap_cnt) {
// TODO WGJA WIP: 	    p->dec_flt = (p->dec_flt * 3) / 4 + p->maj_flt - p->old_maj_flt;
// TODO WGJA WIP: 	    p->old_maj_flt = p->maj_flt;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	    if(p->dec_flt >= SWAP_RATIO / SWAP_MIN) {
// TODO WGJA WIP: 		p->dec_flt = SWAP_RATIO / SWAP_MIN;
// TODO WGJA WIP: 		p->swap_cnt = SWAP_MIN;
// TODO WGJA WIP: 	    } else if(p->dec_flt <= SWAP_RATIO / SWAP_MAX)
// TODO WGJA WIP: 		p->swap_cnt = SWAP_MAX;
// TODO WGJA WIP: 	    else
// TODO WGJA WIP: 		p->swap_cnt = SWAP_RATIO / p->dec_flt;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Go through process' page directory.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	for(table = p->swap_table; table < 1024; table++) {
// TODO WGJA WIP: 	    pg_table = ((unsigned long *) p->tss.cr3)[table];
// TODO WGJA WIP: 	    if(pg_table >= high_memory)
// TODO WGJA WIP: 		    continue;
// TODO WGJA WIP: 	    if(mem_map[MAP_NR(pg_table)] & MAP_PAGE_RESERVED)
// TODO WGJA WIP: 		    continue;
// TODO WGJA WIP: 	    if(!(PAGE_PRESENT & pg_table)) {
// TODO WGJA WIP: 		    printk("swap_out: bad page-table at pg_dir[%d]: %08x\n",
// TODO WGJA WIP: 			    table, pg_table);
// TODO WGJA WIP: 		    ((unsigned long *) p->tss.cr3)[table] = 0;
// TODO WGJA WIP: 		    continue;
// TODO WGJA WIP: 	    }
// TODO WGJA WIP: 	    pg_table &= 0xfffff000;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	    /*
// TODO WGJA WIP: 	     * Go through this page table.
// TODO WGJA WIP: 	     */
// TODO WGJA WIP: 	    for(page = p->swap_page; page < 1024; page++) {
// TODO WGJA WIP: 		switch(try_to_swap_out(page + (unsigned long *) pg_table)) {
// TODO WGJA WIP: 		    case 0:
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 
// TODO WGJA WIP: 		    case 1:
// TODO WGJA WIP: 			p->rss--;
// TODO WGJA WIP: 			/* continue with the following page the next time */
// TODO WGJA WIP: 			p->swap_table = table;
// TODO WGJA WIP: 			p->swap_page  = page + 1;
// TODO WGJA WIP: 			if((--p->swap_cnt) == 0)
// TODO WGJA WIP: 			    swap_task++;
// TODO WGJA WIP: 			return 1;
// TODO WGJA WIP: 
// TODO WGJA WIP: 		    default:
// TODO WGJA WIP: 			p->rss--;
// TODO WGJA WIP: 			break;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	    }
// TODO WGJA WIP: 
// TODO WGJA WIP: 	    p->swap_page = 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * Finish work with this process, if we reached the end of the page
// TODO WGJA WIP: 	 * directory.  Mark restart from the beginning the next time.
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	p->swap_table = 0;
// TODO WGJA WIP:     }
// TODO WGJA WIP:     return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: #else /* old swapping procedure */
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Go through the page tables, searching for a user page that
// TODO WGJA WIP:  * we can swap out.
// TODO WGJA WIP:  * 
// TODO WGJA WIP:  * We now check that the process is swappable (normally only 'init'
// TODO WGJA WIP:  * is un-swappable), allowing high-priority processes which cannot be
// TODO WGJA WIP:  * swapped out (things like user-level device drivers (Not implemented)).
// TODO WGJA WIP:  */
// TODO WGJA WIP: static int swap_out(unsigned int priority)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	static int swap_task = 1;
// TODO WGJA WIP: 	static int swap_table = 0;
// TODO WGJA WIP: 	static int swap_page = 0;
// TODO WGJA WIP: 	int counter = NR_TASKS*8;
// TODO WGJA WIP: 	int pg_table;
// TODO WGJA WIP: 	struct task_struct * p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	counter >>= priority;
// TODO WGJA WIP: check_task:
// TODO WGJA WIP: 	if (counter-- < 0)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (swap_task >= NR_TASKS) {
// TODO WGJA WIP: 		swap_task = 1;
// TODO WGJA WIP: 		goto check_task;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	p = task[swap_task];
// TODO WGJA WIP: 	if (!p || !p->swappable) {
// TODO WGJA WIP: 		swap_task++;
// TODO WGJA WIP: 		goto check_task;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: check_dir:
// TODO WGJA WIP: 	if (swap_table >= PTRS_PER_PAGE) {
// TODO WGJA WIP: 		swap_table = 0;
// TODO WGJA WIP: 		swap_task++;
// TODO WGJA WIP: 		goto check_task;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	pg_table = ((unsigned long *) p->tss.cr3)[swap_table];
// TODO WGJA WIP: 	if (pg_table >= high_memory || (mem_map[MAP_NR(pg_table)] & MAP_PAGE_RESERVED)) {
// TODO WGJA WIP: 		swap_table++;
// TODO WGJA WIP: 		goto check_dir;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!(PAGE_PRESENT & pg_table)) {
// TODO WGJA WIP: 		printk("bad page-table at pg_dir[%d]: %08x\n",
// TODO WGJA WIP: 			swap_table,pg_table);
// TODO WGJA WIP: 		((unsigned long *) p->tss.cr3)[swap_table] = 0;
// TODO WGJA WIP: 		swap_table++;
// TODO WGJA WIP: 		goto check_dir;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	pg_table &= PAGE_MASK;
// TODO WGJA WIP: check_table:
// TODO WGJA WIP: 	if (swap_page >= PTRS_PER_PAGE) {
// TODO WGJA WIP: 		swap_page = 0;
// TODO WGJA WIP: 		swap_table++;
// TODO WGJA WIP: 		goto check_dir;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	switch (try_to_swap_out(swap_page + (unsigned long *) pg_table)) {
// TODO WGJA WIP: 		case 0: break;
// TODO WGJA WIP: 		case 1: p->rss--; return 1;
// TODO WGJA WIP: 		default: p->rss--;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	swap_page++;
// TODO WGJA WIP: 	goto check_table;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 
// TODO WGJA WIP: static int try_to_free_page(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i=6;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	while (i--) {
// TODO WGJA WIP: 		if (shrink_buffers(i))
// TODO WGJA WIP: 			return 1;
// TODO WGJA WIP: 		if (shm_swap(i))
// TODO WGJA WIP: 			return 1;
// TODO WGJA WIP: 		if (swap_out(i))
// TODO WGJA WIP: 			return 1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Note that this must be atomic, or bad things will happen when
// TODO WGJA WIP:  * pages are requested in interrupts (as malloc can do). Thus the
// TODO WGJA WIP:  * cli/sti's.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static inline void add_mem_queue(unsigned long addr, unsigned long * queue)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	addr &= PAGE_MASK;
// TODO WGJA WIP: 	*(unsigned long *) addr = *queue;
// TODO WGJA WIP: 	*queue = addr;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Free_page() adds the page to the free lists. This is optimized for
// TODO WGJA WIP:  * fast normal cases (no error jumps taken normally).
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * The way to optimize jumps for gcc-2.2.2 is to:
// TODO WGJA WIP:  *  - select the "normal" case and put it inside the if () { XXX }
// TODO WGJA WIP:  *  - no else-statements if you can avoid them
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * With the above two rules, you get a straight-line execution path
// TODO WGJA WIP:  * for the normal case, giving better asm-code.
// TODO WGJA WIP:  */
// TODO WGJA WIP: void free_page(unsigned long addr)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (addr < high_memory) {
// TODO WGJA WIP: 		unsigned short * map = mem_map + MAP_NR(addr);
// TODO WGJA WIP: 
// TODO WGJA WIP: 		if (*map) {
// TODO WGJA WIP: 			if (!(*map & MAP_PAGE_RESERVED)) {
// TODO WGJA WIP: 				unsigned long flag;
// TODO WGJA WIP: 
// TODO WGJA WIP: 				save_flags(flag);
// TODO WGJA WIP: 				cli();
// TODO WGJA WIP: 				if (!--*map) {
// TODO WGJA WIP: 					if (nr_secondary_pages < MAX_SECONDARY_PAGES) {
// TODO WGJA WIP: 						add_mem_queue(addr,&secondary_page_list);
// TODO WGJA WIP: 						nr_secondary_pages++;
// TODO WGJA WIP: 						restore_flags(flag);
// TODO WGJA WIP: 						return;
// TODO WGJA WIP: 					}
// TODO WGJA WIP: 					add_mem_queue(addr,&free_page_list);
// TODO WGJA WIP: 					nr_free_pages++;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 				restore_flags(flag);
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		printk("Trying to free free memory (%08x): memory probabably corrupted\n",addr);
// TODO WGJA WIP: 		printk("PC = %08x\n",*(((unsigned long *)&addr)-1));
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This is one ugly macro, but it simplifies checking, and makes
// TODO WGJA WIP:  * this speed-critical place reasonably fast, especially as we have
// TODO WGJA WIP:  * to do things with the interrupt flag etc.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Note that this #define is heavily optimized to give fast code
// TODO WGJA WIP:  * for the normal case - the if-statements are ordered so that gcc-2.2.2
// TODO WGJA WIP:  * will make *no* jumps for the normal code. Don't touch unless you
// TODO WGJA WIP:  * know what you are doing.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define REMOVE_FROM_MEM_QUEUE(queue,nr) \
// TODO WGJA WIP: 	cli(); \
// TODO WGJA WIP: 	if ((result = queue) != 0) { \
// TODO WGJA WIP: 		if (!(result & ~PAGE_MASK) && result < high_memory) { \
// TODO WGJA WIP: 			queue = *(unsigned long *) result; \
// TODO WGJA WIP: 			if (!mem_map[MAP_NR(result)]) { \
// TODO WGJA WIP: 				mem_map[MAP_NR(result)] = 1; \
// TODO WGJA WIP: 				nr--; \
// TODO WGJA WIP: last_free_pages[index = (index + 1) & (NR_LAST_FREE_PAGES - 1)] = result; \
// TODO WGJA WIP: 				restore_flags(flag); \
// TODO WGJA WIP: 				return result; \
// TODO WGJA WIP: 			} \
// TODO WGJA WIP: 			printk("Free page %08x has mem_map = %d\n", \
// TODO WGJA WIP: 				result,mem_map[MAP_NR(result)]); \
// TODO WGJA WIP: 		} else \
// TODO WGJA WIP: 			printk("Result = 0x%08x - memory map destroyed\n", result); \
// TODO WGJA WIP: 		queue = 0; \
// TODO WGJA WIP: 		nr = 0; \
// TODO WGJA WIP: 	} else if (nr) { \
// TODO WGJA WIP: 		printk(#nr " is %d, but " #queue " is empty\n",nr); \
// TODO WGJA WIP: 		nr = 0; \
// TODO WGJA WIP: 	} \
// TODO WGJA WIP: 	restore_flags(flag)
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Get physical address of first (actually last :-) free page, and mark it
// TODO WGJA WIP:  * used. If no free pages left, return 0.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Note that this is one of the most heavily called functions in the kernel,
// TODO WGJA WIP:  * so it's a bit timing-critical (especially as we have to disable interrupts
// TODO WGJA WIP:  * in it). See the above macro which does most of the work, and which is
// TODO WGJA WIP:  * optimized for a fast normal path of execution.
// TODO WGJA WIP:  */
// TODO WGJA WIP: unsigned long __get_free_page(int priority)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long result, flag;
// TODO WGJA WIP: 	static unsigned long index = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* this routine can be called at interrupt time via
// TODO WGJA WIP: 	   malloc.  We want to make sure that the critical
// TODO WGJA WIP: 	   sections of code have interrupts disabled. -RAB
// TODO WGJA WIP: 	   Is this code reentrant? */
// TODO WGJA WIP: 
// TODO WGJA WIP: 	save_flags(flag);
// TODO WGJA WIP: repeat:
// TODO WGJA WIP: 	REMOVE_FROM_MEM_QUEUE(free_page_list,nr_free_pages);
// TODO WGJA WIP: 	if (priority == GFP_BUFFER)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (priority != GFP_ATOMIC)
// TODO WGJA WIP: 		if (try_to_free_page())
// TODO WGJA WIP: 			goto repeat;
// TODO WGJA WIP: 	REMOVE_FROM_MEM_QUEUE(secondary_page_list,nr_secondary_pages);
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
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
