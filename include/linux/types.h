#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

// TODO WGJA WIP: #ifndef _SSIZE_T
// TODO WGJA WIP: #define _SSIZE_T
// TODO WGJA WIP: typedef int ssize_t;
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifndef _TIME_T
// TODO WGJA WIP: #define _TIME_T
// TODO WGJA WIP: typedef long time_t;
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifndef _CLOCK_T
// TODO WGJA WIP: #define _CLOCK_T
// TODO WGJA WIP: typedef long clock_t;
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifndef _PTRDIFF_T
// TODO WGJA WIP: #define _PTRDIFF_T
// TODO WGJA WIP: typedef int ptrdiff_t;
// TODO WGJA WIP: #endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

// TODO WGJA WIP: typedef int pid_t;
// TODO WGJA WIP: typedef unsigned short uid_t;
// TODO WGJA WIP: typedef unsigned short gid_t;
typedef unsigned short dev_t;
// TODO WGJA WIP: #ifdef OLD_LINUX
// TODO WGJA WIP: typedef unsigned short ino_t;
// TODO WGJA WIP: #else
// TODO WGJA WIP: typedef unsigned long ino_t;
// TODO WGJA WIP: #endif
// TODO WGJA WIP: typedef unsigned short mode_t;
// TODO WGJA WIP: typedef unsigned short umode_t;
// TODO WGJA WIP: typedef unsigned short nlink_t;
// TODO WGJA WIP: typedef int daddr_t;
// TODO WGJA WIP: typedef long off_t;
// TODO WGJA WIP: 
// TODO WGJA WIP: /* bsd */
// TODO WGJA WIP: typedef unsigned char u_char;
// TODO WGJA WIP: typedef unsigned short u_short;
// TODO WGJA WIP: typedef unsigned int u_int;
// TODO WGJA WIP: typedef unsigned long u_long;
// TODO WGJA WIP: 
// TODO WGJA WIP: /* sysv */
// TODO WGJA WIP: typedef unsigned char unchar;
// TODO WGJA WIP: typedef unsigned short ushort;
// TODO WGJA WIP: typedef unsigned int uint;
// TODO WGJA WIP: typedef unsigned long ulong;
// TODO WGJA WIP: 
// TODO WGJA WIP: typedef char *caddr_t;
// TODO WGJA WIP: 
// TODO WGJA WIP: typedef unsigned char cc_t;
// TODO WGJA WIP: typedef unsigned int speed_t;
// TODO WGJA WIP: typedef unsigned long tcflag_t;
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * This allows for 256 file descriptors: if NR_OPEN is ever grown beyond that
// TODO WGJA WIP:  * you'll have to change this too. But 256 fd's seem to be enough even for such
// TODO WGJA WIP:  * "real" unices like SunOS, so hopefully this is one limit that doesn't have
// TODO WGJA WIP:  * to be changed.
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Note that POSIX wants the FD_CLEAR(fd,fdsetp) defines to be in <sys/time.h>
// TODO WGJA WIP:  * (and thus <linux/time.h>) - but this is a more logical place for them. Solved
// TODO WGJA WIP:  * by having dummy defines in <sys/time.h>.
// TODO WGJA WIP:  */

/*
 * Those macros may have been defined in <gnu/types.h>. But we always
 * use the ones here. 
 */
#undef __FDSET_LONGS
#define __FDSET_LONGS 8

typedef struct fd_set {
	unsigned long fds_bits [__FDSET_LONGS];
} fd_set;

// TODO WGJA WIP: #undef __NFDBITS
// TODO WGJA WIP: #define __NFDBITS	(8 * sizeof(unsigned long))
// TODO WGJA WIP: 
// TODO WGJA WIP: #undef __FD_SETSIZE
// TODO WGJA WIP: #define __FD_SETSIZE	(__FDSET_LONGS*__NFDBITS)
// TODO WGJA WIP: 
// TODO WGJA WIP: #undef	__FD_SET
// TODO WGJA WIP: #define __FD_SET(fd,fdsetp) \
// TODO WGJA WIP: 		__asm__ __volatile__("btsl %1,%0": \
// TODO WGJA WIP: 			"=m" (*(fd_set *) (fdsetp)):"r" ((int) (fd)))
// TODO WGJA WIP: 
// TODO WGJA WIP: #undef	__FD_CLR
// TODO WGJA WIP: #define __FD_CLR(fd,fdsetp) \
// TODO WGJA WIP: 		__asm__ __volatile__("btrl %1,%0": \
// TODO WGJA WIP: 			"=m" (*(fd_set *) (fdsetp)):"r" ((int) (fd)))
// TODO WGJA WIP: 
// TODO WGJA WIP: #undef	__FD_ISSET
// TODO WGJA WIP: #define __FD_ISSET(fd,fdsetp) (__extension__ ({ \
// TODO WGJA WIP: 		unsigned char __result; \
// TODO WGJA WIP: 		__asm__ __volatile__("btl %1,%2 ; setb %0" \
// TODO WGJA WIP: 			:"=q" (__result) :"r" ((int) (fd)), \
// TODO WGJA WIP: 			"m" (*(fd_set *) (fdsetp))); \
// TODO WGJA WIP: 		__result; }))
// TODO WGJA WIP: 
// TODO WGJA WIP: #undef	__FD_ZERO
// TODO WGJA WIP: #define __FD_ZERO(fdsetp) \
// TODO WGJA WIP: 		__asm__ __volatile__("cld ; rep ; stosl" \
// TODO WGJA WIP: 			:"=m" (*(fd_set *) (fdsetp)) \
// TODO WGJA WIP: 			:"a" (0), "c" (__FDSET_LONGS), \
// TODO WGJA WIP: 			"D" ((fd_set *) (fdsetp)) :"cx","di")
// TODO WGJA WIP: 
// TODO WGJA WIP: struct ustat {
// TODO WGJA WIP: 	daddr_t f_tfree;
// TODO WGJA WIP: 	ino_t f_tinode;
// TODO WGJA WIP: 	char f_fname[6];
// TODO WGJA WIP: 	char f_fpack[6];
// TODO WGJA WIP: };

#endif
