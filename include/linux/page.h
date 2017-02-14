#ifndef _LINUX_PAGE_H
#define _LINUX_PAGE_H

			/* PAGE_SHIFT determines the page size */
#define PAGE_SHIFT			12
#define PAGE_SIZE			((unsigned long)1<<PAGE_SHIFT)
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifdef __KERNEL__
// TODO WGJA WIP: 
// TODO WGJA WIP: 			/* number of bits that fit into a memory pointer */
// TODO WGJA WIP: #define BITS_PER_PTR			(8*sizeof(unsigned long))
// TODO WGJA WIP: 			/* to mask away the intra-page address bits */
// TODO WGJA WIP: #define PAGE_MASK			(~(PAGE_SIZE-1))
// TODO WGJA WIP: 			/* to align the pointer to the (next) page boundary */
// TODO WGJA WIP: #define PAGE_ALIGN(addr)		(((addr)+PAGE_SIZE-1)&PAGE_MASK)
// TODO WGJA WIP: 			/* to align the pointer to a pointer address */
// TODO WGJA WIP: #define PTR_MASK			(~(sizeof(void*)-1))
// TODO WGJA WIP: 
// TODO WGJA WIP: 					/* sizeof(void*)==1<<SIZEOF_PTR_LOG2 */
// TODO WGJA WIP: 					/* 64-bit machines, beware!  SRB. */
// TODO WGJA WIP: #define SIZEOF_PTR_LOG2			2
// TODO WGJA WIP: 
// TODO WGJA WIP: 			/* to find an entry in a page-table-directory */
// TODO WGJA WIP: #define PAGE_DIR_OFFSET(base,address)	((unsigned long*)((base)+\
// TODO WGJA WIP:   ((unsigned long)(address)>>(PAGE_SHIFT-SIZEOF_PTR_LOG2)*2&PTR_MASK&~PAGE_MASK)))
// TODO WGJA WIP: 			/* to find an entry in a page-table */
// TODO WGJA WIP: #define PAGE_PTR(address)		\
// TODO WGJA WIP:   ((unsigned long)(address)>>PAGE_SHIFT-SIZEOF_PTR_LOG2&PTR_MASK&~PAGE_MASK)
// TODO WGJA WIP: 			/* the no. of pointers that fit on a page */
// TODO WGJA WIP: #define PTRS_PER_PAGE			(PAGE_SIZE/sizeof(void*))
// TODO WGJA WIP: 
// TODO WGJA WIP: #endif /* __KERNEL__ */
// TODO WGJA WIP: 
#endif /* _LINUX_PAGE_H */
