#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <features.h>
#include <sys/types.h>
#include <linux/signal.h>

#ifndef SIGCLD
#define SIGCLD		SIGCHLD
#endif

typedef int sig_atomic_t;

typedef __sighandler_t	SignalHandler;

#ifndef BADSIG
#define BADSIG		SIG_ERR
#endif

/* The Interviews version also has these ... */

#define SignalBad	((SignalHandler)-1)
#define SignalDefault	((SignalHandler)0)
#define SignalIgnore	((SignalHandler)1)

__BEGIN_DECLS

#ifdef __STDC__
extern const char * const sys_siglist[];
extern const char * const _sys_siglist[];
#else
extern char* sys_siglist[];
extern char* _sys_siglist[];
#endif

extern __sighandler_t
		signal __P ((int __sig, __sighandler_t));
extern int	raise __P ((int __sig));
extern int	__kill __P ((pid_t __pid, int __sig));
extern int	kill __P ((pid_t __pid, int __sig));
extern int	killpg __P ((int __pgrp, int __sig));
extern int	sigaddset __P ((sigset_t *__mask, int __sig));
extern int	sigdelset __P ((sigset_t *__mask, int __sig));
extern int	sigemptyset __P ((sigset_t *__mask));
extern int	sigfillset __P ((sigset_t *__mask));
extern int	sigismember __P ((__const sigset_t *__mask, int __sig));
extern int	sigpending __P ((sigset_t *__set));

extern int	__sigprocmask __P ((int __how, sigset_t *__set,
			sigset_t *__oldset));
extern int	sigprocmask __P ((int __how, sigset_t *__set,
			sigset_t *__oldset));

extern int	sigsuspend __P ((sigset_t *sigmask));

extern int	__sigaction __P ((int __sig, struct sigaction *__act,
			struct sigaction *__oldact));
extern int	sigaction __P ((int __sig, struct sigaction *__act,
			struct sigaction *__oldact));

#define __sigemptyset(set)	((*(set) = 0L), 0)
#define __sigfillset(set)       ((*(set) = -1L), 0)
#define __sigaddset(set, sig)   ((*(set) |= __sigmask (sig)), 0)
#define __sigdelset(set, sig)   ((*(set) &= ~__sigmask (sig)), 0)
#define __sigismember(set, sig) ((*(set) & __sigmask (sig)) ? 1 : 0)


#ifdef __OPTIMIZE__
#define sigemptyset	__sigemptyset
#define sigfillset	__sigfillset

/* We don't do that any more since it causes problems due to
 * "sig" > _NSIG and "sig" < 1. It isn't worth the touble to make
 * them inline and static. Use __sigxxxxx if you want speed with
 * correct "sig".
 */
#if 0
#define sigaddset	__sigaddset
#define sigdelset	__sigdelset
#define sigismember	__sigismember
#endif

#endif


/* Return a mask that includes SIG only.  */
#define __sigmask(sig)	(1 << ((sig) - 1))

extern sigset_t	__sigsetmask __P ((sigset_t __mask));
extern sigset_t	__siggetmask __P ((void));
extern int	__sigblock __P ((int __mask));
extern int	__sigpause __P ((int __mask));

#ifdef  __USE_SVID
/* SVID names for the same things.  */
extern __sighandler_t ssignal __P ((int __sig, __sighandler_t __handler));
extern int gsignal __P ((int __sig));

#endif /* Use SVID.  */
 
/* BSD */
#ifdef __USE_BSD
#define sigmask		__sigmask

extern int	sigblock __P ((int __mask));
extern int	sigpause __P ((int __mask));
extern sigset_t	sigsetmask __P ((int __mask));
extern sigset_t	siggetmask __P ((void));
extern void	psignal __P ((int __sig, __const char *__str));

extern int	siginterrupt __P ((int __sig, int __flag));

/* The `sig' bit is set if the interrupt on it
 * is enabled via siginterrupt (). */
extern sigset_t _sigintr;

#endif  /* Use BSD.  */

#ifdef __USE_BSD_SIGNAL

extern __sighandler_t
		__bsd_signal __P ((int __sig, __sighandler_t));
#define signal	__bsd_signal

#endif	/* __USE_BSD_SIGNAL */

__END_DECLS

#endif /* _SIGNAL_H */
