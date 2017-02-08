/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	POSIX Standard: 5.1.2 Directory Operations	<dirent.h>
 */

#ifndef	_DIRENT_H

#define	_DIRENT_H	1
#include <features.h>

__BEGIN_DECLS

#include <gnu/types.h>

#define	__need_size_t
#include <stddef.h>

#include <sys/types.h>
#include <linux/limits.h>
#include <linux/dirent.h>

#if defined(__USE_GNU)
#define	d_fileno	d_ino		/* glibc compatibility.  */
#define	d_namlen	d_reclen	/* glibc compatibility.  */
#endif

/* For now, syscall readdir () only supports one entry at a time. It
 * will be changed in the future.
#define NUMENT		3
*/
#ifndef NUMENT
#define NUMENT		1
#endif

/* Directory stream type.  */
typedef struct {
  int dd_fd;			/* file descriptor */
  int dd_loc;			/* offset in buffer */
  int dd_size;			/* # of valid entries in buffer */
  struct dirent *dd_buf;	/* -> directory buffer */
} DIR;				/* stream data from opendir() */


/* Open a directory stream on NAME.
   Return a DIR stream on the directory, or NULL if it could not be opened.  */
extern DIR *opendir __P ((__const char *__name));

/* Close the directory stream DIRP.
   Return 0 if successful, -1 if not.  */
extern int closedir __P ((DIR * __dirp));

/* Read a directory entry from DIRP.
   Return a pointer to a `struct dirent' describing the entry,
   or NULL for EOF or error.  The storage returned may be overwritten
   by a later readdir call on the same DIR stream.  */
extern struct dirent *readdir __P ((DIR * __dirp));

/* Rewind DIRP to the beginning of the directory.  */
extern void rewinddir __P ((DIR * __dirp));

#if defined(__USE_BSD) || defined(__USE_MISC)

#ifndef	MAXNAMLEN
/* Get the definitions of the POSIX.1 limits.  */
#include <posix1_lim.h>

/* `MAXNAMLEN' is the BSD name for what POSIX calls `NAME_MAX'.  */
#ifdef	NAME_MAX
#define	MAXNAMLEN	NAME_MAX
#else
#define	MAXNAMLEN	255
#endif
#endif

#include <gnu/types.h>

/* Seek to position POS on DIRP.  */
extern void seekdir __P ((DIR * __dirp, __off_t __pos));

/* Return the current position of DIRP.  */
extern __off_t telldir __P ((DIR * __dirp));

typedef int (*__dir_select_fn_t) __P ((__const struct dirent *));

typedef int (*__dir_compar_fn_t) __P ((
		__const struct dirent * __const *,
		__const struct dirent * __const *
		));

/* Scan the directory DIR, calling SELECT on each directory entry.
   Entries for which SELECT returns nonzero are individually malloc'd,
   sorted using qsort with CMP, and collected in a malloc'd array in
   *NAMELIST.  Returns the number of entries selected, or -1 on error.  */
extern int scandir __P ((__const char *__dir,
			 struct dirent ***__namelist,
			 __dir_select_fn_t __select,
			 __dir_compar_fn_t __compar));

/* Function to compare two `struct dirent's alphabetically.  */
extern int alphasort __P ((
		__const struct dirent * __const *,
		__const struct dirent * __const *
		));


/* Read directory entries from FD into BUF, reading at most NBYTES.
   Reading starts at offset *BASEP, and *BASEP is updated with the new
   position after reading.  Returns the number of bytes read; zero when at
   end of directory; or -1 for errors.  */
extern __ssize_t __getdirentries __P ((int __fd, char *__buf,
				       size_t __nbytes, __off_t *__basep));
extern __ssize_t getdirentries __P ((int __fd, char *__buf,
				     size_t __nbytes, __off_t *__basep));


#endif /* Use BSD or misc.  */

__END_DECLS

#endif /* dirent.h  */
