/*
 *  linux/mm/memory.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * demand-loading started 01.12.91 - seems it is high on the list of
 * things wanted, and it should be easy to implement. - Linus
 */

/*
 * Ok, demand-loading was easy, shared pages a little bit tricker. Shared
 * pages started 02.12.91, seems to work. - Linus.
 *
 * Tested sharing by executing about 30 /bin/sh: under the old kernel it
 * would have taken more than the 6M I have free, but it worked well as
 * far as I could see.
 *
 * Also corrected some "invalidate()"s - I wasn't doing enough of them.
 */

/*
 * Real VM (paging to/from disk) started 18.12.91. Much more work and
 * thought has to go into this. Oh, well..
 * 19.12.91  -  works, somewhat. Sometimes I get faults, don't know why.
 *		Found it. Everything seems to work now.
 * 20.12.91  -  Ok, making the swap-device changeable like the root.
 */

#include <asm/system.h>

#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>

unsigned long high_memory = 0;

extern void sound_mem_init(void);
// TODO WGJA WIP: extern void die_if_kernel(char *,struct pt_regs *,long);

int nr_swap_pages = 0;
int nr_free_pages = 0;
unsigned long free_page_list = 0;
/*
 * The secondary free_page_list is used for malloc() etc things that
 * may need pages during interrupts etc. Normal get_free_page() operations
 * don't touch it, so it stays as a kind of "panic-list", that can be
 * accessed when all other mm tricks have failed.
 */
int nr_secondary_pages = 0;
unsigned long secondary_page_list = 0;

#define copy_page(to,from)	memcpy((void *)(to), (void *)(from), PAGE_SIZE)

unsigned short * mem_map = (unsigned short *) NULL;

#define CODE_SPACE(addr,p) ((addr) < (p)->end_code)

/*
 * oom() prints a message (so that the user knows why the process died),
 * and gives the process an untrappable SIGSEGV.
 */
void oom(struct task_struct * task)
{
	printk("\nout of memory\n");
	task->sigaction[SIGKILL-1].sa_handler = NULL;
	task->blocked &= ~(1<<(SIGKILL-1));
	send_sig(SIGKILL,task,1);
}

static void free_one_table(unsigned long * page_dir)
{
	int j;
	unsigned long pg_table = *page_dir;
	unsigned long * page_table;

	if (!pg_table)
		return;
	*page_dir = 0;
	if (pg_table >= high_memory || !(pg_table & PAGE_PRESENT)) {
		printk("Bad page table: [%08x]=%08x\n",page_dir,pg_table);
		return;
	}
	if (mem_map[MAP_NR(pg_table)] & MAP_PAGE_RESERVED)
		return;
	page_table = (unsigned long *) (pg_table & PAGE_MASK);
	for (j = 0 ; j < PTRS_PER_PAGE ; j++,page_table++) {
		unsigned long pg = *page_table;

		if (!pg)
			continue;
		*page_table = 0;
		if (pg & PAGE_PRESENT)
			free_page(PAGE_MASK & pg);
		else
			swap_free(pg);
	}
	free_page(PAGE_MASK & pg_table);
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This function clears all user-level page tables of a process - this
// TODO WGJA WIP:  * is needed by execve(), so that old pages aren't in the way. Note that
// TODO WGJA WIP:  * unlike 'free_page_tables()', this function still leaves a valid
// TODO WGJA WIP:  * page-table-tree in memory: it just removes the user pages. The two
// TODO WGJA WIP:  * functions are similar, but there is a fundamental difference.
// TODO WGJA WIP:  */
// TODO WGJA WIP: void clear_page_tables(struct task_struct * tsk)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 	unsigned long pg_dir;
// TODO WGJA WIP: 	unsigned long * page_dir;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!tsk)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (tsk == task[0])
// TODO WGJA WIP: 		panic("task[0] (swapper) doesn't support exec()\n");
// TODO WGJA WIP: 	pg_dir = tsk->tss.cr3;
// TODO WGJA WIP: 	page_dir = (unsigned long *) pg_dir;
// TODO WGJA WIP: 	if (!page_dir || page_dir == swapper_pg_dir) {
// TODO WGJA WIP: 		printk("Trying to clear kernel page-directory: not good\n");
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (mem_map[MAP_NR(pg_dir)] > 1) {
// TODO WGJA WIP: 		unsigned long * new_pg;
// TODO WGJA WIP: 
// TODO WGJA WIP: 		if (!(new_pg = (unsigned long*) get_free_page(GFP_KERNEL))) {
// TODO WGJA WIP: 			oom(tsk);
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		for (i = 768 ; i < 1024 ; i++)
// TODO WGJA WIP: 			new_pg[i] = page_dir[i];
// TODO WGJA WIP: 		free_page(pg_dir);
// TODO WGJA WIP: 		tsk->tss.cr3 = (unsigned long) new_pg;
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	for (i = 0 ; i < 768 ; i++,page_dir++)
// TODO WGJA WIP: 		free_one_table(page_dir);
// TODO WGJA WIP: 	invalidate();
// TODO WGJA WIP: 	return;
// TODO WGJA WIP: }

/*
 * This function frees up all page tables of a process when it exits.
 */
void free_page_tables(struct task_struct * tsk)
{
	int i;
	unsigned long pg_dir;
	unsigned long * page_dir;

	if (!tsk)
		return;
	if (tsk == task[0]) {
		printk("task[0] (swapper) killed: unable to recover\n");
		panic("Trying to free up swapper memory space");
	}
	pg_dir = tsk->tss.cr3;
	if (!pg_dir || pg_dir == (unsigned long) swapper_pg_dir) {
		printk("Trying to free kernel page-directory: not good\n");
		return;
	}
	tsk->tss.cr3 = (unsigned long) swapper_pg_dir;
	if (tsk == current)
		__asm__ __volatile__("movl %0,%%cr3": :"a" (tsk->tss.cr3));
	if (mem_map[MAP_NR(pg_dir)] > 1) {
		free_page(pg_dir);
		return;
	}
	page_dir = (unsigned long *) pg_dir;
	for (i = 0 ; i < PTRS_PER_PAGE ; i++,page_dir++)
		free_one_table(page_dir);
	free_page(pg_dir);
	invalidate();
}

/*
 * clone_page_tables() clones the page table for a process - both
 * processes will have the exact same pages in memory. There are
 * probably races in the memory management with cloning, but we'll
 * see..
 */
int clone_page_tables(struct task_struct * tsk)
{
	unsigned long pg_dir;

	pg_dir = current->tss.cr3;
	mem_map[MAP_NR(pg_dir)]++;
	tsk->tss.cr3 = pg_dir;
	return 0;
}

/*
 * copy_page_tables() just copies the whole process memory range:
 * note the special handling of RESERVED (ie kernel) pages, which
 * means that they are always shared by all processes.
 */
int copy_page_tables(struct task_struct * tsk)
{
	int i;
	unsigned long old_pg_dir, *old_page_dir;
	unsigned long new_pg_dir, *new_page_dir;

	if (!(new_pg_dir = get_free_page(GFP_KERNEL)))
		return -ENOMEM;
	old_pg_dir = current->tss.cr3;
	tsk->tss.cr3 = new_pg_dir;
	old_page_dir = (unsigned long *) old_pg_dir;
	new_page_dir = (unsigned long *) new_pg_dir;
	for (i = 0 ; i < PTRS_PER_PAGE ; i++,old_page_dir++,new_page_dir++) {
		int j;
		unsigned long old_pg_table, *old_page_table;
		unsigned long new_pg_table, *new_page_table;

		old_pg_table = *old_page_dir;
		if (!old_pg_table)
			continue;
		if (old_pg_table >= high_memory || !(old_pg_table & PAGE_PRESENT)) {
			printk("copy_page_tables: bad page table: "
				"probable memory corruption");
			*old_page_dir = 0;
			continue;
		}
		if (mem_map[MAP_NR(old_pg_table)] & MAP_PAGE_RESERVED) {
			*new_page_dir = old_pg_table;
			continue;
		}
		if (!(new_pg_table = get_free_page(GFP_KERNEL))) {
			free_page_tables(tsk);
			return -ENOMEM;
		}
		old_page_table = (unsigned long *) (PAGE_MASK & old_pg_table);
		new_page_table = (unsigned long *) (PAGE_MASK & new_pg_table);
		for (j = 0 ; j < PTRS_PER_PAGE ; j++,old_page_table++,new_page_table++) {
			unsigned long pg;
			pg = *old_page_table;
			if (!pg)
				continue;
			if (!(pg & PAGE_PRESENT)) {
				*new_page_table = swap_duplicate(pg);
				continue;
			}
			if ((pg & (PAGE_RW | PAGE_COW)) == (PAGE_RW | PAGE_COW))
				pg &= ~PAGE_RW;
			*new_page_table = pg;
			if (mem_map[MAP_NR(pg)] & MAP_PAGE_RESERVED)
				continue;
			*old_page_table = pg;
			mem_map[MAP_NR(pg)]++;
		}
		*new_page_dir = new_pg_table | PAGE_TABLE;
	}
	invalidate();
	return 0;
}

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * a more complete version of free_page_tables which performs with page
// TODO WGJA WIP:  * granularity.
// TODO WGJA WIP:  */
// TODO WGJA WIP: int unmap_page_range(unsigned long from, unsigned long size)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long page, page_dir;
// TODO WGJA WIP: 	unsigned long *page_table, *dir;
// TODO WGJA WIP: 	unsigned long poff, pcnt, pc;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (from & ~PAGE_MASK) {
// TODO WGJA WIP: 		printk("unmap_page_range called with wrong alignment\n");
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	size = (size + ~PAGE_MASK) >> PAGE_SHIFT;
// TODO WGJA WIP: 	dir = PAGE_DIR_OFFSET(current->tss.cr3,from);
// TODO WGJA WIP: 	poff = (from >> PAGE_SHIFT) & PTRS_PER_PAGE-1;
// TODO WGJA WIP: 	if ((pcnt = PTRS_PER_PAGE - poff) > size)
// TODO WGJA WIP: 		pcnt = size;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	for ( ; size > 0; ++dir, size -= pcnt,
// TODO WGJA WIP: 	     pcnt = (size > PTRS_PER_PAGE ? PTRS_PER_PAGE : size)) {
// TODO WGJA WIP: 		if (!(page_dir = *dir))	{
// TODO WGJA WIP: 			poff = 0;
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (!(page_dir & PAGE_PRESENT)) {
// TODO WGJA WIP: 			printk("unmap_page_range: bad page directory.");
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		page_table = (unsigned long *)(PAGE_MASK & page_dir);
// TODO WGJA WIP: 		if (poff) {
// TODO WGJA WIP: 			page_table += poff;
// TODO WGJA WIP: 			poff = 0;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		for (pc = pcnt; pc--; page_table++) {
// TODO WGJA WIP: 			if ((page = *page_table) != 0) {
// TODO WGJA WIP: 				*page_table = 0;
// TODO WGJA WIP: 				if (1 & page) {
// TODO WGJA WIP: 					if (!(mem_map[MAP_NR(page)]
// TODO WGJA WIP: 					      & MAP_PAGE_RESERVED))
// TODO WGJA WIP: 						--current->rss;
// TODO WGJA WIP: 					free_page(PAGE_MASK & page);
// TODO WGJA WIP: 				} else
// TODO WGJA WIP: 					swap_free(page);
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (pcnt == PTRS_PER_PAGE) {
// TODO WGJA WIP: 			*dir = 0;
// TODO WGJA WIP: 			free_page(PAGE_MASK & page_dir);
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	invalidate();
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: int zeromap_page_range(unsigned long from, unsigned long size, int mask)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long *page_table, *dir;
// TODO WGJA WIP: 	unsigned long poff, pcnt;
// TODO WGJA WIP: 	unsigned long page;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (mask) {
// TODO WGJA WIP: 		if ((mask & (PAGE_MASK|PAGE_PRESENT)) != PAGE_PRESENT) {
// TODO WGJA WIP: 			printk("zeromap_page_range: mask = %08x\n",mask);
// TODO WGJA WIP: 			return -EINVAL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		mask |= ZERO_PAGE;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (from & ~PAGE_MASK) {
// TODO WGJA WIP: 		printk("zeromap_page_range: from = %08x\n",from);
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	dir = PAGE_DIR_OFFSET(current->tss.cr3,from);
// TODO WGJA WIP: 	size = (size + ~PAGE_MASK) >> PAGE_SHIFT;
// TODO WGJA WIP: 	poff = (from >> PAGE_SHIFT) & PTRS_PER_PAGE-1;
// TODO WGJA WIP: 	if ((pcnt = PTRS_PER_PAGE - poff) > size)
// TODO WGJA WIP: 		pcnt = size;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	while (size > 0) {
// TODO WGJA WIP: 		if (!(PAGE_PRESENT & *dir)) {
// TODO WGJA WIP: 				/* clear page needed here?  SRB. */
// TODO WGJA WIP: 			if (!(page_table = (unsigned long*) get_free_page(GFP_KERNEL))) {
// TODO WGJA WIP: 				invalidate();
// TODO WGJA WIP: 				return -ENOMEM;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			if (PAGE_PRESENT & *dir) {
// TODO WGJA WIP: 				free_page((unsigned long) page_table);
// TODO WGJA WIP: 				page_table = (unsigned long *)(PAGE_MASK & *dir++);
// TODO WGJA WIP: 			} else
// TODO WGJA WIP: 				*dir++ = ((unsigned long) page_table) | PAGE_TABLE;
// TODO WGJA WIP: 		} else
// TODO WGJA WIP: 			page_table = (unsigned long *)(PAGE_MASK & *dir++);
// TODO WGJA WIP: 		page_table += poff;
// TODO WGJA WIP: 		poff = 0;
// TODO WGJA WIP: 		for (size -= pcnt; pcnt-- ;) {
// TODO WGJA WIP: 			if ((page = *page_table) != 0) {
// TODO WGJA WIP: 				*page_table = 0;
// TODO WGJA WIP: 				if (page & PAGE_PRESENT) {
// TODO WGJA WIP: 					if (!(mem_map[MAP_NR(page)]
// TODO WGJA WIP: 					      & MAP_PAGE_RESERVED))
// TODO WGJA WIP: 						--current->rss;
// TODO WGJA WIP: 					free_page(PAGE_MASK & page);
// TODO WGJA WIP: 				} else
// TODO WGJA WIP: 					swap_free(page);
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			*page_table++ = mask;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		pcnt = (size > PTRS_PER_PAGE ? PTRS_PER_PAGE : size);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	invalidate();
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * maps a range of physical memory into the requested pages. the old
// TODO WGJA WIP:  * mappings are removed. any references to nonexistent pages results
// TODO WGJA WIP:  * in null mappings (currently treated as "copy-on-access")
// TODO WGJA WIP:  */
// TODO WGJA WIP: int remap_page_range(unsigned long from, unsigned long to, unsigned long size, int mask)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long *page_table, *dir;
// TODO WGJA WIP: 	unsigned long poff, pcnt;
// TODO WGJA WIP: 	unsigned long page;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (mask) {
// TODO WGJA WIP: 		if ((mask & (PAGE_MASK|PAGE_PRESENT)) != PAGE_PRESENT) {
// TODO WGJA WIP: 			printk("remap_page_range: mask = %08x\n",mask);
// TODO WGJA WIP: 			return -EINVAL;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if ((from & ~PAGE_MASK) || (to & ~PAGE_MASK)) {
// TODO WGJA WIP: 		printk("remap_page_range: from = %08x, to=%08x\n",from,to);
// TODO WGJA WIP: 		return -EINVAL;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	dir = PAGE_DIR_OFFSET(current->tss.cr3,from);
// TODO WGJA WIP: 	size = (size + ~PAGE_MASK) >> PAGE_SHIFT;
// TODO WGJA WIP: 	poff = (from >> PAGE_SHIFT) & PTRS_PER_PAGE-1;
// TODO WGJA WIP: 	if ((pcnt = PTRS_PER_PAGE - poff) > size)
// TODO WGJA WIP: 		pcnt = size;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	while (size > 0) {
// TODO WGJA WIP: 		if (!(PAGE_PRESENT & *dir)) {
// TODO WGJA WIP: 			/* clearing page here, needed?  SRB. */
// TODO WGJA WIP: 			if (!(page_table = (unsigned long*) get_free_page(GFP_KERNEL))) {
// TODO WGJA WIP: 				invalidate();
// TODO WGJA WIP: 				return -1;
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			*dir++ = ((unsigned long) page_table) | PAGE_TABLE;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			page_table = (unsigned long *)(PAGE_MASK & *dir++);
// TODO WGJA WIP: 		if (poff) {
// TODO WGJA WIP: 			page_table += poff;
// TODO WGJA WIP: 			poff = 0;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 
// TODO WGJA WIP: 		for (size -= pcnt; pcnt-- ;) {
// TODO WGJA WIP: 			if ((page = *page_table) != 0) {
// TODO WGJA WIP: 				*page_table = 0;
// TODO WGJA WIP: 				if (PAGE_PRESENT & page) {
// TODO WGJA WIP: 					if (!(mem_map[MAP_NR(page)]
// TODO WGJA WIP: 					      & MAP_PAGE_RESERVED))
// TODO WGJA WIP: 						--current->rss;
// TODO WGJA WIP: 					free_page(PAGE_MASK & page);
// TODO WGJA WIP: 				} else
// TODO WGJA WIP: 					swap_free(page);
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 
// TODO WGJA WIP: 			/*
// TODO WGJA WIP: 			 * the first condition should return an invalid access
// TODO WGJA WIP: 			 * when the page is referenced. current assumptions
// TODO WGJA WIP: 			 * cause it to be treated as demand allocation in some
// TODO WGJA WIP: 			 * cases.
// TODO WGJA WIP: 			 */
// TODO WGJA WIP: 			if (!mask)
// TODO WGJA WIP: 				*page_table++ = 0;	/* not present */
// TODO WGJA WIP: 			else if (to >= high_memory)
// TODO WGJA WIP: 				*page_table++ = (to | mask);
// TODO WGJA WIP: 			else if (!mem_map[MAP_NR(to)])
// TODO WGJA WIP: 				*page_table++ = 0;	/* not present */
// TODO WGJA WIP: 			else {
// TODO WGJA WIP: 				*page_table++ = (to | mask);
// TODO WGJA WIP: 				if (!(mem_map[MAP_NR(to)] & MAP_PAGE_RESERVED)) {
// TODO WGJA WIP: 					++current->rss;
// TODO WGJA WIP: 					mem_map[MAP_NR(to)]++;
// TODO WGJA WIP: 				}
// TODO WGJA WIP: 			}
// TODO WGJA WIP: 			to += PAGE_SIZE;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		pcnt = (size > PTRS_PER_PAGE ? PTRS_PER_PAGE : size);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	invalidate();
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This function puts a page in memory at the wanted address.
// TODO WGJA WIP:  * It returns the physical address of the page gotten, 0 if
// TODO WGJA WIP:  * out of memory (either when trying to access page-table or
// TODO WGJA WIP:  * page.)
// TODO WGJA WIP:  */
// TODO WGJA WIP: unsigned long put_page(struct task_struct * tsk,unsigned long page,
// TODO WGJA WIP: 	unsigned long address,int prot)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long *page_table;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if ((prot & (PAGE_MASK|PAGE_PRESENT)) != PAGE_PRESENT)
// TODO WGJA WIP: 		printk("put_page: prot = %08x\n",prot);
// TODO WGJA WIP: 	if (page >= high_memory) {
// TODO WGJA WIP: 		printk("put_page: trying to put page %p at %p\n",page,address);
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	page_table = PAGE_DIR_OFFSET(tsk->tss.cr3,address);
// TODO WGJA WIP: 	if ((*page_table) & PAGE_PRESENT)
// TODO WGJA WIP: 		page_table = (unsigned long *) (PAGE_MASK & *page_table);
// TODO WGJA WIP: 	else {
// TODO WGJA WIP: 		printk("put_page: bad page directory entry\n");
// TODO WGJA WIP: 		oom(tsk);
// TODO WGJA WIP: 		*page_table = BAD_PAGETABLE | PAGE_TABLE;
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	page_table += (address >> PAGE_SHIFT) & PTRS_PER_PAGE-1;
// TODO WGJA WIP: 	if (*page_table) {
// TODO WGJA WIP: 		printk("put_page: page already exists\n");
// TODO WGJA WIP: 		*page_table = 0;
// TODO WGJA WIP: 		invalidate();
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	*page_table = page | prot;
// TODO WGJA WIP: /* no need for invalidate */
// TODO WGJA WIP: 	return page;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * The previous function doesn't work very well if you also want to mark
// TODO WGJA WIP:  * the page dirty: exec.c wants this, as it has earlier changed the page,
// TODO WGJA WIP:  * and we want the dirty-status to be correct (for VM). Thus the same
// TODO WGJA WIP:  * routine, but this time we mark it dirty too.
// TODO WGJA WIP:  */
// TODO WGJA WIP: unsigned long put_dirty_page(struct task_struct * tsk, unsigned long page, unsigned long address)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long tmp, *page_table;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (page >= high_memory)
// TODO WGJA WIP: 		printk("put_dirty_page: trying to put page %p at %p\n",page,address);
// TODO WGJA WIP: 	if (mem_map[MAP_NR(page)] != 1)
// TODO WGJA WIP: 		printk("mem_map disagrees with %p at %p\n",page,address);
// TODO WGJA WIP: 	page_table = PAGE_DIR_OFFSET(tsk->tss.cr3,address);
// TODO WGJA WIP: 	if (PAGE_PRESENT & *page_table)
// TODO WGJA WIP: 		page_table = (unsigned long *) (PAGE_MASK & *page_table);
// TODO WGJA WIP: 	else {
// TODO WGJA WIP: 		if (!(tmp = get_free_page(GFP_KERNEL)))
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		if (PAGE_PRESENT & *page_table) {
// TODO WGJA WIP: 			free_page(tmp);
// TODO WGJA WIP: 			page_table = (unsigned long *) (PAGE_MASK & *page_table);
// TODO WGJA WIP: 		} else {
// TODO WGJA WIP: 			*page_table = tmp | PAGE_TABLE;
// TODO WGJA WIP: 			page_table = (unsigned long *) tmp;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	page_table += (address >> PAGE_SHIFT) & PTRS_PER_PAGE-1;
// TODO WGJA WIP: 	if (*page_table) {
// TODO WGJA WIP: 		printk("put_dirty_page: page already exists\n");
// TODO WGJA WIP: 		*page_table = 0;
// TODO WGJA WIP: 		invalidate();
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	*page_table = page | (PAGE_DIRTY | PAGE_PRIVATE);
// TODO WGJA WIP: /* no need for invalidate */
// TODO WGJA WIP: 	return page;
// TODO WGJA WIP: }

/*
 * This routine handles present pages, when users try to write
 * to a shared page. It is done by copying the page to a new address
 * and decrementing the shared-page counter for the old page.
 *
 * Note that we do many checks twice (look at do_wp_page()), as
 * we have to be careful about race-conditions.
 *
 * Goto-purists beware: the only reason for goto's here is that it results
 * in better assembly code.. The "default" path will see no jumps at all.
 */
static void __do_wp_page(unsigned long error_code, unsigned long address,
	struct task_struct * tsk, unsigned long user_esp)
{
	unsigned long *pde, pte, old_page, prot;
	unsigned long new_page;

	new_page = __get_free_page(GFP_KERNEL);
	pde = PAGE_DIR_OFFSET(tsk->tss.cr3,address);
	pte = *pde;
	if (!(pte & PAGE_PRESENT))
		goto end_wp_page;
	if ((pte & PAGE_TABLE) != PAGE_TABLE || pte >= high_memory)
		goto bad_wp_pagetable;
	pte &= PAGE_MASK;
	pte += PAGE_PTR(address);
	old_page = *(unsigned long *) pte;
	if (!(old_page & PAGE_PRESENT))
		goto end_wp_page;
	if (old_page >= high_memory)
		goto bad_wp_page;
	if (old_page & PAGE_RW)
		goto end_wp_page;
	tsk->min_flt++;
	prot = (old_page & ~PAGE_MASK) | PAGE_RW;
	old_page &= PAGE_MASK;
	if (mem_map[MAP_NR(old_page)] != 1) {
		if (new_page) {
			if (mem_map[MAP_NR(old_page)] & MAP_PAGE_RESERVED)
				++tsk->rss;
			copy_page(old_page,new_page);
			*(unsigned long *) pte = new_page | prot;
			free_page(old_page);
			invalidate();
			return;
		}
		free_page(old_page);
		oom(tsk);
		*(unsigned long *) pte = BAD_PAGE | prot;
		invalidate();
		return;
	}
	*(unsigned long *) pte |= PAGE_RW;
	invalidate();
	if (new_page)
		free_page(new_page);
	return;
bad_wp_page:
	printk("do_wp_page: bogus page at address %08x (%08x)\n",address,old_page);
	*(unsigned long *) pte = BAD_PAGE | PAGE_SHARED;
	send_sig(SIGKILL, tsk, 1);
	goto end_wp_page;
bad_wp_pagetable:
	printk("do_wp_page: bogus page-table at address %08x (%08x)\n",address,pte);
	*pde = BAD_PAGETABLE | PAGE_TABLE;
	send_sig(SIGKILL, tsk, 1);
end_wp_page:
	if (new_page)
		free_page(new_page);
	return;
}

/*
 * check that a page table change is actually needed, and call
 * the low-level function only in that case..
 */
void do_wp_page(unsigned long error_code, unsigned long address,
	struct task_struct * tsk, unsigned long user_esp)
{
	unsigned long page;
	unsigned long * pg_table;

	pg_table = PAGE_DIR_OFFSET(tsk->tss.cr3,address);
	page = *pg_table;
	if (!page)
		return;
	if ((page & PAGE_PRESENT) && page < high_memory) {
		pg_table = (unsigned long *) ((page & PAGE_MASK) + PAGE_PTR(address));
		page = *pg_table;
		if (!(page & PAGE_PRESENT))
			return;
		if (page & PAGE_RW)
			return;
		if (!(page & PAGE_COW)) {
			if (user_esp && tsk == current) {
				send_sig(SIGSEGV, tsk, 1);
				return;
			}
		}
		if (mem_map[MAP_NR(page)] == 1) {
			*pg_table |= PAGE_RW | PAGE_DIRTY;
			invalidate();
			return;
		}
		__do_wp_page(error_code, address, tsk, user_esp);
		return;
	}
	printk("bad page directory entry %08x\n",page);
	*pg_table = 0;
}

int verify_area(int type, void * addr, unsigned long size)
{
	unsigned long start;

	start = (unsigned long) addr;
	if (start >= TASK_SIZE)
		return -EFAULT;
	if (size > TASK_SIZE - start)
		return -EFAULT;
	if (type == VERIFY_READ || !size)
		return 0;
	if (!size)
		return 0;
	size--;
	size += start & ~PAGE_MASK;
	size >>= PAGE_SHIFT;
	start &= PAGE_MASK;
	do {
		do_wp_page(1,start,current,0);
		start += PAGE_SIZE;
	} while (size--);
	return 0;
}

// TODO WGJA WIP: static inline void get_empty_page(struct task_struct * tsk, unsigned long address)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long tmp;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!(tmp = get_free_page(GFP_KERNEL))) {
// TODO WGJA WIP: 		oom(tsk);
// TODO WGJA WIP: 		tmp = BAD_PAGE;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (!put_page(tsk,tmp,address,PAGE_PRIVATE))
// TODO WGJA WIP: 		free_page(tmp);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * try_to_share() checks the page at address "address" in the task "p",
// TODO WGJA WIP:  * to see if it exists, and if it is clean. If so, share it with the current
// TODO WGJA WIP:  * task.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * NOTE! This assumes we have checked that p != current, and that they
// TODO WGJA WIP:  * share the same executable or library.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * We may want to fix this to allow page sharing for PIC pages at different
// TODO WGJA WIP:  * addresses so that ELF will really perform properly. As long as the vast
// TODO WGJA WIP:  * majority of sharable libraries load at fixed addresses this is not a
// TODO WGJA WIP:  * big concern. Any sharing of pages between the buffer cache and the
// TODO WGJA WIP:  * code space reduces the need for this as well.  - ERY
// TODO WGJA WIP:  */
// TODO WGJA WIP: static int try_to_share(unsigned long address, struct task_struct * tsk,
// TODO WGJA WIP: 	struct task_struct * p, unsigned long error_code, unsigned long newpage)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long from;
// TODO WGJA WIP: 	unsigned long to;
// TODO WGJA WIP: 	unsigned long from_page;
// TODO WGJA WIP: 	unsigned long to_page;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	from_page = (unsigned long)PAGE_DIR_OFFSET(p->tss.cr3,address);
// TODO WGJA WIP: 	to_page = (unsigned long)PAGE_DIR_OFFSET(tsk->tss.cr3,address);
// TODO WGJA WIP: /* is there a page-directory at from? */
// TODO WGJA WIP: 	from = *(unsigned long *) from_page;
// TODO WGJA WIP: 	if (!(from & PAGE_PRESENT))
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	from &= PAGE_MASK;
// TODO WGJA WIP: 	from_page = from + PAGE_PTR(address);
// TODO WGJA WIP: 	from = *(unsigned long *) from_page;
// TODO WGJA WIP: /* is the page clean and present? */
// TODO WGJA WIP: 	if ((from & (PAGE_PRESENT | PAGE_DIRTY)) != PAGE_PRESENT)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (from >= high_memory)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (mem_map[MAP_NR(from)] & MAP_PAGE_RESERVED)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: /* is the destination ok? */
// TODO WGJA WIP: 	to = *(unsigned long *) to_page;
// TODO WGJA WIP: 	if (!(to & PAGE_PRESENT))
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	to &= PAGE_MASK;
// TODO WGJA WIP: 	to_page = to + PAGE_PTR(address);
// TODO WGJA WIP: 	if (*(unsigned long *) to_page)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: /* share them if read - do COW immediately otherwise */
// TODO WGJA WIP: 	if (error_code & PAGE_RW) {
// TODO WGJA WIP: 		if(!newpage)	/* did the page exist?  SRB. */
// TODO WGJA WIP: 			return 0;
// TODO WGJA WIP: 		copy_page((from & PAGE_MASK),newpage);
// TODO WGJA WIP: 		to = newpage | PAGE_PRIVATE;
// TODO WGJA WIP: 	} else {
// TODO WGJA WIP: 		mem_map[MAP_NR(from)]++;
// TODO WGJA WIP: 		from &= ~PAGE_RW;
// TODO WGJA WIP: 		to = from;
// TODO WGJA WIP: 		if(newpage)	/* only if it existed. SRB. */
// TODO WGJA WIP: 			free_page(newpage);
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	*(unsigned long *) from_page = from;
// TODO WGJA WIP: 	*(unsigned long *) to_page = to;
// TODO WGJA WIP: 	invalidate();
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * share_page() tries to find a process that could share a page with
// TODO WGJA WIP:  * the current one. Address is the address of the wanted page relative
// TODO WGJA WIP:  * to the current data space.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * We first check if it is at all feasible by checking executable->i_count.
// TODO WGJA WIP:  * It should be >1 if there are other tasks sharing this inode.
// TODO WGJA WIP:  */
// TODO WGJA WIP: int share_page(struct vm_area_struct * area, struct task_struct * tsk,
// TODO WGJA WIP: 	struct inode * inode,
// TODO WGJA WIP: 	unsigned long address, unsigned long error_code, unsigned long newpage)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct task_struct ** p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!inode || inode->i_count < 2)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
// TODO WGJA WIP: 		if (!*p)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (tsk == *p)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (inode != (*p)->executable) {
// TODO WGJA WIP: 			  if(!area) continue;
// TODO WGJA WIP: 			/* Now see if there is something in the VMM that
// TODO WGJA WIP: 			   we can share pages with */
// TODO WGJA WIP: 			if(area){
// TODO WGJA WIP: 			  struct vm_area_struct * mpnt;
// TODO WGJA WIP: 			  for(mpnt = (*p)->mmap; mpnt; mpnt = mpnt->vm_next){
// TODO WGJA WIP: 			    if(mpnt->vm_ops && mpnt->vm_ops == area->vm_ops &&
// TODO WGJA WIP: 			       mpnt->vm_inode->i_ino == area->vm_inode->i_ino&&
// TODO WGJA WIP: 			       mpnt->vm_inode->i_dev == area->vm_inode->i_dev){
// TODO WGJA WIP: 			      if (mpnt->vm_ops->share(mpnt, area, address))
// TODO WGJA WIP: 				break;
// TODO WGJA WIP: 			    };
// TODO WGJA WIP: 			  };
// TODO WGJA WIP: 			  if (!mpnt) continue;  /* Nope.  Nuthin here */
// TODO WGJA WIP: 			};
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (try_to_share(address,tsk,*p,error_code,newpage))
// TODO WGJA WIP: 			return 1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * fill in an empty page-table if none exists.
// TODO WGJA WIP:  */
// TODO WGJA WIP: static inline unsigned long get_empty_pgtable(struct task_struct * tsk,unsigned long address)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long page;
// TODO WGJA WIP: 	unsigned long *p;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	p = PAGE_DIR_OFFSET(tsk->tss.cr3,address);
// TODO WGJA WIP: 	if (PAGE_PRESENT & *p)
// TODO WGJA WIP: 		return *p;
// TODO WGJA WIP: 	if (*p) {
// TODO WGJA WIP: 		printk("get_empty_pgtable: bad page-directory entry \n");
// TODO WGJA WIP: 		*p = 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	page = get_free_page(GFP_KERNEL);
// TODO WGJA WIP: 	p = PAGE_DIR_OFFSET(tsk->tss.cr3,address);
// TODO WGJA WIP: 	if (PAGE_PRESENT & *p) {
// TODO WGJA WIP: 		free_page(page);
// TODO WGJA WIP: 		return *p;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (*p) {
// TODO WGJA WIP: 		printk("get_empty_pgtable: bad page-directory entry \n");
// TODO WGJA WIP: 		*p = 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (page) {
// TODO WGJA WIP: 		*p = page | PAGE_TABLE;
// TODO WGJA WIP: 		return *p;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	oom(current);
// TODO WGJA WIP: 	*p = BAD_PAGETABLE | PAGE_TABLE;
// TODO WGJA WIP: 	return 0;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void do_no_page(unsigned long error_code, unsigned long address,
// TODO WGJA WIP: 	struct task_struct *tsk, unsigned long user_esp)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long tmp;
// TODO WGJA WIP: 	unsigned long page;
// TODO WGJA WIP: 	struct vm_area_struct * mpnt;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	page = get_empty_pgtable(tsk,address);
// TODO WGJA WIP: 	if (!page)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	page &= PAGE_MASK;
// TODO WGJA WIP: 	page += PAGE_PTR(address);
// TODO WGJA WIP: 	tmp = *(unsigned long *) page;
// TODO WGJA WIP: 	if (tmp & PAGE_PRESENT)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	++tsk->rss;
// TODO WGJA WIP: 	if (tmp) {
// TODO WGJA WIP: 		++tsk->maj_flt;
// TODO WGJA WIP: 		swap_in((unsigned long *) page);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	address &= 0xfffff000;
// TODO WGJA WIP: 	for (mpnt = tsk->mmap ; mpnt ; mpnt = mpnt->vm_next) {
// TODO WGJA WIP: 		if (address < mpnt->vm_start)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		if (address >= ((mpnt->vm_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)))
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		mpnt->vm_ops->nopage(error_code, mpnt, address);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	++tsk->min_flt;
// TODO WGJA WIP: 	get_empty_page(tsk,address);
// TODO WGJA WIP: 	if (tsk != current)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (address < tsk->brk)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	if (address+8192 >= (user_esp & 0xfffff000) && 
// TODO WGJA WIP: 	    address <= current->start_stack)
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	send_sig(SIGSEGV,tsk,1);
// TODO WGJA WIP: 	return;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This routine handles page faults.  It determines the address,
// TODO WGJA WIP:  * and the problem, and then passes it off to one of the appropriate
// TODO WGJA WIP:  * routines.
// TODO WGJA WIP:  */
// TODO WGJA WIP: extern "C" void do_page_fault(struct pt_regs *regs, unsigned long error_code)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	unsigned long address;
// TODO WGJA WIP: 	unsigned long user_esp = 0;
// TODO WGJA WIP: 	unsigned long stack_limit;
// TODO WGJA WIP: 	unsigned int bit;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/* get the address */
// TODO WGJA WIP: 	__asm__("movl %%cr2,%0":"=r" (address));
// TODO WGJA WIP: 	if (address < TASK_SIZE) {
// TODO WGJA WIP: 		if (error_code & 4) {	/* user mode access? */
// TODO WGJA WIP: 			if (regs->eflags & VM_MASK) {
// TODO WGJA WIP: 				bit = (address - 0xA0000) >> PAGE_SHIFT;
// TODO WGJA WIP: 				if (bit < 32)
// TODO WGJA WIP: 					current->screen_bitmap |= 1 << bit;
// TODO WGJA WIP: 			} else 
// TODO WGJA WIP: 				user_esp = regs->esp;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		if (error_code & 1)
// TODO WGJA WIP: 			do_wp_page(error_code, address, current, user_esp);
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			do_no_page(error_code, address, current, user_esp);
// TODO WGJA WIP: 		if (!user_esp)
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		stack_limit = current->rlim[RLIMIT_STACK].rlim_cur;
// TODO WGJA WIP: 		if (stack_limit >= RLIM_INFINITY)
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		if (stack_limit >= current->start_stack)
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 		stack_limit = current->start_stack - stack_limit;
// TODO WGJA WIP: 		if (user_esp < stack_limit)
// TODO WGJA WIP: 			send_sig(SIGSEGV, current, 1);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	printk("Unable to handle kernel paging request at address %08x\n",address);
// TODO WGJA WIP: 	die_if_kernel("Oops", regs, error_code);
// TODO WGJA WIP: 	do_exit(SIGKILL);
// TODO WGJA WIP: }

/*
 * BAD_PAGE is the page that is used for page faults when linux
 * is out-of-memory. Older versions of linux just did a
 * do_exit(), but using this instead means there is less risk
 * for a process dying in kernel mode, possibly leaving a inode
 * unused etc..
 *
 * BAD_PAGETABLE is the accompanying page-table: it is initialized
 * to point to BAD_PAGE entries.
 *
 * ZERO_PAGE is a special page that is used for zero-initialized
 * data and COW.
 */
unsigned long __bad_pagetable(void)
{
	extern char empty_bad_page_table[PAGE_SIZE];

	__asm__ __volatile__("cld ; rep ; stosl":
		:"a" (BAD_PAGE + PAGE_TABLE),
		 "D" ((long) empty_bad_page_table),
		 "c" (PTRS_PER_PAGE)
		:"di","cx");
	return (unsigned long) empty_bad_page_table;
}

unsigned long __bad_page(void)
{
	printk("TODO __bad_page\n");for(;;); // TODO WGJA __bad_page
	extern char empty_bad_page[PAGE_SIZE];

	// __asm__ __volatile__("cld ; rep ; stosl":
	// 	:"a" (0),
	// 	 "D" ((long) empty_bad_page),
	// 	 "c" (PTRS_PER_PAGE)
	// 	:"di","cx");
	return (unsigned long) empty_bad_page;
}

// TODO WGJA WIP: unsigned long __zero_page(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	extern char empty_zero_page[PAGE_SIZE];
// TODO WGJA WIP: 
// TODO WGJA WIP: 	__asm__ __volatile__("cld ; rep ; stosl":
// TODO WGJA WIP: 		:"a" (0),
// TODO WGJA WIP: 		 "D" ((long) empty_zero_page),
// TODO WGJA WIP: 		 "c" (PTRS_PER_PAGE)
// TODO WGJA WIP: 		:"di","cx");
// TODO WGJA WIP: 	return (unsigned long) empty_zero_page;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void show_mem(void)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i,free = 0,total = 0,reserved = 0;
// TODO WGJA WIP: 	int shared = 0;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	printk("Mem-info:\n");
// TODO WGJA WIP: 	printk("Free pages:      %6dkB\n",nr_free_pages<<PAGE_SHIFT-10);
// TODO WGJA WIP: 	printk("Secondary pages: %6dkB\n",nr_secondary_pages<<PAGE_SHIFT-10);
// TODO WGJA WIP: 	printk("Free swap:       %6dkB\n",nr_swap_pages<<PAGE_SHIFT-10);
// TODO WGJA WIP: 	printk("Buffer memory:   %6dkB\n",buffermem>>10);
// TODO WGJA WIP: 	printk("Buffer heads:    %6d\n",nr_buffer_heads);
// TODO WGJA WIP: 	printk("Buffer blocks:   %6d\n",nr_buffers);
// TODO WGJA WIP: 	i = high_memory >> PAGE_SHIFT;
// TODO WGJA WIP: 	while (i-- > 0) {
// TODO WGJA WIP: 		total++;
// TODO WGJA WIP: 		if (mem_map[i] & MAP_PAGE_RESERVED)
// TODO WGJA WIP: 			reserved++;
// TODO WGJA WIP: 		else if (!mem_map[i])
// TODO WGJA WIP: 			free++;
// TODO WGJA WIP: 		else
// TODO WGJA WIP: 			shared += mem_map[i]-1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	printk("%d pages of RAM\n",total);
// TODO WGJA WIP: 	printk("%d free pages\n",free);
// TODO WGJA WIP: 	printk("%d reserved pages\n",reserved);
// TODO WGJA WIP: 	printk("%d pages shared\n",shared);
// TODO WGJA WIP: }

/*
 * paging_init() sets up the page tables - note that the first 4MB are
 * already mapped by head.S.
 *
 * This routines also unmaps the page at virtual kernel address 0, so
 * that we can trap those pesky NULL-reference errors in the kernel.
 */
unsigned long paging_init(unsigned long start_mem, unsigned long end_mem)
{
	unsigned long * pg_dir;
	unsigned long * pg_table;
	unsigned long tmp;
	unsigned long address;

/*
 * Physical page 0 is special: it's a "zero-page", and is guaranteed to
 * stay that way - it's write-protected and when there is a c-o-w, the
 * mm handler treats it specially.
 */
	memset((void *) 0, 0, PAGE_SIZE);
	start_mem = PAGE_ALIGN(start_mem);
	address = 0;
	pg_dir = swapper_pg_dir;
	while (address < end_mem) {
		tmp = *(pg_dir + 768);		/* at virtual addr 0xC0000000 */
		if (!tmp) {
			tmp = start_mem | PAGE_TABLE;
			*(pg_dir + 768) = tmp;
			start_mem += PAGE_SIZE;
		}
		*pg_dir = tmp;			/* also map it in at 0x0000000 for init */
		pg_dir++;
		pg_table = (unsigned long *) (tmp & PAGE_MASK);
		for (tmp = 0 ; tmp < PTRS_PER_PAGE ; tmp++,pg_table++) {
			if (address && address < end_mem)
				*pg_table = address | PAGE_SHARED;
			else
				*pg_table = 0;
			address += PAGE_SIZE;
		}
	}
	invalidate();
	return start_mem;
}

void mem_init(unsigned long start_low_mem,
	      unsigned long start_mem, unsigned long end_mem)
{
	int codepages = 0;
	int reservedpages = 0;
	int datapages = 0;
	unsigned long tmp;
	unsigned short * p;
	extern int etext;

	cli();
	end_mem &= PAGE_MASK;
	high_memory = end_mem;
	start_mem +=  0x0000000f;
	start_mem &= ~0x0000000f;
	tmp = MAP_NR(end_mem);
	mem_map = (unsigned short *) start_mem;
	p = mem_map + tmp;
	start_mem = (unsigned long) p;
	while (p > mem_map)
		*--p = MAP_PAGE_RESERVED;
	start_low_mem = PAGE_ALIGN(start_low_mem);
	start_mem = PAGE_ALIGN(start_mem);
	while (start_low_mem < 0xA0000) {
		mem_map[MAP_NR(start_low_mem)] = 0;
		start_low_mem += PAGE_SIZE;
	}
	while (start_mem < end_mem) {
		mem_map[MAP_NR(start_mem)] = 0;
		start_mem += PAGE_SIZE;
	}
	sound_mem_init();
	free_page_list = 0;
	nr_free_pages = 0;
	for (tmp = 0 ; tmp < end_mem ; tmp += PAGE_SIZE) {
		if (mem_map[MAP_NR(tmp)]) {
			if (tmp >= 0xA0000 && tmp < 0x100000)
				reservedpages++;
			else if (tmp < (unsigned long) &etext)
				codepages++;
			else
				datapages++;
			continue;
		}
		*(unsigned long *) tmp = free_page_list;
		free_page_list = tmp;
		nr_free_pages++;
	}
	tmp = nr_free_pages << PAGE_SHIFT;
	printk("Memory: %dk/%dk available (%dk kernel code, %dk reserved, %dk data)\n",
		tmp >> 10,
		end_mem >> 10,
		codepages << PAGE_SHIFT-10,
		reservedpages << PAGE_SHIFT-10,
		datapages << PAGE_SHIFT-10);
	return;
}

// TODO WGJA WIP: void si_meminfo(struct sysinfo *val)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	int i;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	i = high_memory >> PAGE_SHIFT;
// TODO WGJA WIP: 	val->totalram = 0;
// TODO WGJA WIP: 	val->freeram = 0;
// TODO WGJA WIP: 	val->sharedram = 0;
// TODO WGJA WIP: 	val->bufferram = buffermem;
// TODO WGJA WIP: 	while (i-- > 0)  {
// TODO WGJA WIP: 		if (mem_map[i] & MAP_PAGE_RESERVED)
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		val->totalram++;
// TODO WGJA WIP: 		if (!mem_map[i]) {
// TODO WGJA WIP: 			val->freeram++;
// TODO WGJA WIP: 			continue;
// TODO WGJA WIP: 		}
// TODO WGJA WIP: 		val->sharedram += mem_map[i]-1;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	val->totalram <<= PAGE_SHIFT;
// TODO WGJA WIP: 	val->freeram <<= PAGE_SHIFT;
// TODO WGJA WIP: 	val->sharedram <<= PAGE_SHIFT;
// TODO WGJA WIP: 	return;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: 
// TODO WGJA WIP: /* This handles a generic mmap of a disk file */
// TODO WGJA WIP: void file_mmap_nopage(int error_code, struct vm_area_struct * area, unsigned long address)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	struct inode * inode = area->vm_inode;
// TODO WGJA WIP: 	unsigned int block;
// TODO WGJA WIP: 	unsigned int clear;
// TODO WGJA WIP: 	unsigned long page;
// TODO WGJA WIP: 	unsigned long tmp;
// TODO WGJA WIP: 	int nr[8];
// TODO WGJA WIP: 	int i, j;
// TODO WGJA WIP: 	int prot = area->vm_page_prot; /* prot for buffer cache.. */
// TODO WGJA WIP: 
// TODO WGJA WIP: 	address &= PAGE_MASK;
// TODO WGJA WIP: 	block = address - area->vm_start + area->vm_offset;
// TODO WGJA WIP: 	block >>= inode->i_sb->s_blocksize_bits;
// TODO WGJA WIP: 
// TODO WGJA WIP: 	page = get_free_page(GFP_KERNEL);
// TODO WGJA WIP: 	if (share_page(area, area->vm_task, inode, address, error_code, page)) {
// TODO WGJA WIP: 		++area->vm_task->min_flt;
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	++area->vm_task->maj_flt;
// TODO WGJA WIP: 	if (!page) {
// TODO WGJA WIP: 		oom(current);
// TODO WGJA WIP: 		put_page(area->vm_task, BAD_PAGE, address, PAGE_PRIVATE);
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	for (i=0, j=0; i< PAGE_SIZE ; j++, block++, i += inode->i_sb->s_blocksize)
// TODO WGJA WIP: 		nr[j] = bmap(inode,block);
// TODO WGJA WIP: 
// TODO WGJA WIP: 	/*
// TODO WGJA WIP: 	 * If we don't mmap a whole page, we have to clear the end of the page,
// TODO WGJA WIP: 	 * which also means that we can't share the page with the buffer cache.
// TODO WGJA WIP: 	 * This is easy to handle by giving the 'bread_page()' a protection mask
// TODO WGJA WIP: 	 * that contains PAGE_RW, as the cache code won't try to share then..
// TODO WGJA WIP: 	 */
// TODO WGJA WIP: 	clear = 0;
// TODO WGJA WIP: 	if (address + PAGE_SIZE > area->vm_end) {
// TODO WGJA WIP: 		clear = address + PAGE_SIZE - area->vm_end;
// TODO WGJA WIP: 		prot |= PAGE_RW;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	page = bread_page(page, inode->i_dev, nr, inode->i_sb->s_blocksize, prot);
// TODO WGJA WIP: 
// TODO WGJA WIP: 	if (!(error_code & PAGE_RW)) {
// TODO WGJA WIP: 		if (share_page(area, area->vm_task, inode, address, error_code, page))
// TODO WGJA WIP: 			return;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 
// TODO WGJA WIP: 	tmp = page + PAGE_SIZE;
// TODO WGJA WIP: 	while (clear--) {
// TODO WGJA WIP: 		*(char *)--tmp = 0;
// TODO WGJA WIP: 	}
// TODO WGJA WIP: 	if (put_page(area->vm_task,page,address,area->vm_page_prot))
// TODO WGJA WIP: 		return;
// TODO WGJA WIP: 	free_page(page);
// TODO WGJA WIP: 	oom(current);
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: void file_mmap_free(struct vm_area_struct * area)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (area->vm_inode)
// TODO WGJA WIP: 		iput(area->vm_inode);
// TODO WGJA WIP: #if 0
// TODO WGJA WIP: 	if (area->vm_inode)
// TODO WGJA WIP: 		printk("Free inode %x:%d (%d)\n",area->vm_inode->i_dev, 
// TODO WGJA WIP: 				 area->vm_inode->i_ino, area->vm_inode->i_count);
// TODO WGJA WIP: #endif
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Compare the contents of the mmap entries, and decide if we are allowed to
// TODO WGJA WIP:  * share the pages
// TODO WGJA WIP:  */
// TODO WGJA WIP: int file_mmap_share(struct vm_area_struct * area1, 
// TODO WGJA WIP: 		    struct vm_area_struct * area2, 
// TODO WGJA WIP: 		    unsigned long address)
// TODO WGJA WIP: {
// TODO WGJA WIP: 	if (area1->vm_inode != area2->vm_inode)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (area1->vm_start != area2->vm_start)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (area1->vm_end != area2->vm_end)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (area1->vm_offset != area2->vm_offset)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	if (area1->vm_page_prot != area2->vm_page_prot)
// TODO WGJA WIP: 		return 0;
// TODO WGJA WIP: 	return 1;
// TODO WGJA WIP: }
// TODO WGJA WIP: 
// TODO WGJA WIP: struct vm_operations_struct file_mmap = {
// TODO WGJA WIP: 	NULL,			/* open */
// TODO WGJA WIP: 	file_mmap_free,		/* close */
// TODO WGJA WIP: 	file_mmap_nopage,	/* nopage */
// TODO WGJA WIP: 	NULL,			/* wppage */
// TODO WGJA WIP: 	file_mmap_share,	/* share */
// TODO WGJA WIP: };
