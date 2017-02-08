/* Declarations for `malloc' and friends.
   Copyright 1990, 1991, 1992 Free Software Foundation, Inc.
		  Written May 1989 by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

#ifndef _MALLOC_H
#define _MALLOC_H	1

#include <features.h>

#ifndef	NULL
#ifdef __cplusplus
#define	NULL	0
#else
#define	NULL	((void *) 0)
#endif
#endif

#ifdef	__STDC__
#include <stddef.h>
#else
#undef	size_t
#define	size_t		unsigned int
#undef	ptrdiff_t
#define	ptrdiff_t	int
#endif

/* For backward compatibilities and X11R5 */
#if (defined(MALLOC_0_RETURNS_NULL) || defined(NO_FIX_MALLOC)) \
	&& !defined(__MALLOC_0_RETURNS_NULL)
#define __MALLOC_0_RETURNS_NULL
#endif
 
__BEGIN_DECLS

#if defined(_STDLIB_H) && !defined(__MALLOC_0_RETURNS_NULL)
/* Allocate SIZE bytes of memory.  */
static __inline void * malloc __P ((size_t __size));
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
static __inline void * calloc __P ((size_t __nmemb, size_t __size));
#else
/* Allocate SIZE bytes of memory.  */
extern __ptr_t malloc __P ((size_t __size));
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern __ptr_t calloc __P ((size_t __nmemb, size_t __size));
#endif
/* Re-allocate the previously allocated block
   in __ptr_t, making the new block SIZE bytes long.  */
extern __ptr_t realloc __P ((__ptr_t __ptr, size_t __size));
/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free __P ((__ptr_t __ptr));

/* Allocate SIZE bytes allocated to ALIGNMENT bytes.  */
extern __ptr_t memalign __P ((size_t __alignment, size_t __size));

/* Allocate SIZE bytes on a page boundary.  */
extern __ptr_t valloc __P ((size_t __size));

/* Underlying allocation function; successive calls should
   return contiguous pieces of memory.  */
extern __ptr_t (*__morecore) __P ((ptrdiff_t __size));

/* Default value of `__morecore'.  */
extern __ptr_t __default_morecore __P ((ptrdiff_t __size));

/* Nonzero if `malloc' has been called and done its initialization.  */
extern int __malloc_initialized;

/* Hooks for debugging versions.  */
extern void (*__free_hook) __P ((__ptr_t __ptr));
extern __ptr_t (*__malloc_hook) __P ((size_t __size));
extern __ptr_t (*__realloc_hook) __P ((__ptr_t __ptr, size_t __size));

/* Activate a standard collection of debugging hooks.  */
extern void mcheck __P ((void (*__func) __P ((void))));

/* Activate a standard collection of tracing hooks.  */
extern void mtrace __P ((void));

/* Statistics available to the user.  */
struct mstats
  {
    size_t bytes_total;		/* Total size of the heap. */
    size_t chunks_used;		/* Chunks allocated by the user. */
    size_t bytes_used;		/* Byte total of user-allocated chunks. */
    size_t chunks_free;		/* Chunks in the free list. */
    size_t bytes_free;		/* Byte total of chunks in the free list. */
  };

/* Pick up the current statistics. */
extern struct mstats mstats __P ((void));

__END_DECLS


#endif /* malloc.h  */
