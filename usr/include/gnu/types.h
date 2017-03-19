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

#ifndef	_GNU_TYPES_H

#define	_GNU_TYPES_H	1


/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short __u_short;
typedef unsigned int __u_int;
typedef unsigned long __u_long;
typedef struct
{
  long val[2];
} __quad;
typedef struct
{
  __u_long val[2];
} __u_quad;

#ifdef __linux__
typedef unsigned short __dev_t;	/* Type of device numbers.  */
typedef unsigned short __gid_t;	/* Type of group identifications.  */
typedef unsigned short __uid_t;	/* Type of user identifications.  */
typedef unsigned short __mode_t;/* Type of file attribute bitmasks.  */
typedef long	__daddr_t;	/* The type of a disk address.  */
typedef long	__off_t;	/* Type of file sizes and offsets.  */
typedef unsigned long __ino_t;	/* Type of file serial numbers.  */
typedef unsigned short __nlink_t;	/* Type of file link counts.  */
typedef long	__time_t;
#else
typedef int __dev_t;		/* Type of device numbers.  */
typedef unsigned int __gid_t;	/* Type of group identifications.  */
typedef unsigned int __uid_t;	/* Type of user identifications.  */
typedef unsigned int __mode_t;	/* Type of file attribute bitmasks.  */
typedef long int __daddr_t;	/* The type of a disk address.  */
typedef long int __off_t;	/* Type of file sizes and offsets.  */
typedef unsigned long int __ino_t;	/* Type of file serial numbers.  */
typedef unsigned short int __nlink_t;	/* Type of file link counts.  */
typedef long int __time_t;
#endif

typedef int __pid_t;		/* Type of process identifications.  */
typedef int __ssize_t;		/* Type of a byte count, or error.  */
typedef __quad __fsid_t;	/* Type of file system IDs.  */

/* Everythin' else.  */
typedef char *__caddr_t;
typedef long int __swblk_t;	/* Type of a swap block maybe?  */


/* fd_set for select.  */

#ifdef __linux__

#ifndef __FDSET_LONGS
#define __FDSET_LONGS 8
#endif

typedef struct __fd_set {
        unsigned long fds_bits [__FDSET_LONGS];
} __fd_set;


#ifndef __NFDBITS

#define	__NFDBITS	(sizeof(unsigned long int) * 8)

#define __FD_SETSIZE	(__FDSET_LONGS*__NFDBITS)

#define __FD_SET(fd,fdsetp) \
	__asm__ __volatile__ ("btsl %1,%0": \
		"=m" (*(__fd_set *) (fdsetp)):"r" ((int) (fd)))

#define __FD_CLR(fd,fdsetp) \
	__asm__ __volatile__("btrl %1,%0": \
		"=m" (*(__fd_set *) (fdsetp)):"r" ((int) (fd)))

#define __FD_ISSET(fd,fdsetp) __extension__ ({ \
	unsigned char __result; \
	__asm__ __volatile__("btl %1,%2 ; setb %0" \
        	:"=q" (__result) :"r" ((int) (fd)), \
		"m" (*(__fd_set *) (fdsetp))); \
	__result; })

#undef	__FD_ZERO
#define __FD_ZERO(fdsetp) \
do { \
	int __d0, __d1; \
	__asm__ __volatile__("cld ; rep ; stosl" \
			:"=m" (*(fd_set*) fdsetp), \
			  "=&c" (__d0), "=&D" (__d1) \
			:"a" (0), "1" (__FDSET_LONGS), \
			"2" ((fd_set*) fdsetp) : "memory"); \
} while (0)

#endif /* __NFDBITS */

#else /* __linux__ */

/* Number of descriptors that can fit in an `fd_set'.  */
#define	__FD_SETSIZE	256

/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define	__NFDBITS	(sizeof(unsigned long int) * 8)
#define	__FDELT(d)	((d) / __NFDBITS)
#define	__FDMASK(d)	(1 << ((d) % __NFDBITS))

typedef struct
{
  unsigned long int fds_bits [(__FD_SETSIZE + (__NFDBITS - 1)) / __NFDBITS];
} __fd_set;

/* This line MUST be split!  Otherwise m4 will not change it.  */
#define	__FD_ZERO(set)	\
  ((void) memset((__ptr_t) (set), 0, sizeof(fd_set)))
#define	__FD_SET(d, set)	((set)->__bits[__FDELT(d)] |= __FDMASK(d))
#define	__FD_CLR(d, set)	((set)->__bits[__FDELT(d)] &= ~__FDMASK(d))
#define	__FD_ISSET(d, set)	((set)->__bits[__FDELT(d)] & __FDMASK(d))

#endif /* __linux__ */

#endif /* gnu/types.h */
