#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <sys/cdefs.h>

#define __WANT_POSIX1B_SIGNALS__

#include <sys/types.h>
#include <endian.h>

#ifdef __mips__
#define _NSIG		128
#else
#define _NSIG		64
#endif

#define NSIG		32
#define SIGRTMAX	(_NSIG-1)
#define MINSIGSTKSZ	2048
#define _NSIG_WORDS	((_NSIG/sizeof(long))>>3)

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGSEGV		11
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGUNUSED	31
#if defined(__i386__) || defined(__x86_64__) || defined(__powerpc__) || defined(__arm__) \
	|| defined(__s390__) || defined(__ia64__) || defined(__powerpc64__)
#define SIGBUS		 7
#define SIGUSR1		10
#define SIGUSR2		12
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGURG		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGIO		29
#define SIGPWR		30
#define SIGSYS		31
#elif defined(__alpha__) || defined(__sparc__)
#define SIGEMT		 7
#define SIGBUS		10
#define SIGSYS		12
#define SIGURG		16
#define SIGSTOP		17
#define SIGTSTP		18
#define SIGCONT		19
#define SIGCHLD		20
#define SIGTTIN		21
#define SIGTTOU		22
#define SIGIO		23
#define SIGXCPU		24
#define SIGXFSZ		25
#define SIGVTALRM	26
#define SIGPROF		27
#define SIGWINCH	28
#define SIGPWR		29
#define SIGUSR1		30
#define SIGUSR2		31
#if defined(__alpha__)
#define SIGINFO		SIGPWR
#endif
#elif defined(__mips__)
#define SIGEMT		 7
#define SIGBUS		10
#define SIGSYS		12
#define SIGUSR1		16
#define SIGUSR2		17
#define SIGCHLD		18
#define SIGPWR		19
#define SIGWINCH	20
#define SIGURG		21
#define SIGIO		22
#define SIGSTOP		23
#define SIGTSTP		24
#define SIGCONT		25
#define SIGTTIN		26
#define SIGTTOU		27
#define SIGVTALRM	28
#define SIGPROF		29
#define SIGXCPU		30
#define SIGXFSZ		31
#elif defined(__hppa__)
#define SIGEMT		 7
#define SIGBUS		10
#define SIGSYS		12
#define SIGUSR1		16
#define SIGUSR2		17
#define SIGCHLD		18
#define SIGPWR		19
#define SIGVTALRM	20
#define SIGPROF		21
#define SIGIO		22
#define SIGWINCH	23
#define SIGSTOP		24
#define SIGTSTP		25
#define SIGCONT		26
#define SIGTTIN		27
#define SIGTTOU		28
#define SIGURG		29
#define SIGLOST		30
#define SIGUNUSED	31
#define SIGRESERVE	SIGUNUSE
#define SIGXCPU		33
#define SIGXFSZ		34
#define SIGSTKFLT	36

#else
#error signal layout not yet known
#endif

#define SIGCLD		SIGCHLD
#define SIGPOLL		SIGIO

/* These should not be considered constants from userland.  */
#ifdef __hppa__
#define SIGRTMIN	37
#else
#define SIGLOST		SIGPWR
#define SIGRTMIN	32
#endif
#define SIGRTMAX	(_NSIG-1)

/* SA_FLAGS values: */
#if defined(__alpha__)
#define SA_ONSTACK	0x00000001
#define SA_RESTART	0x00000002
#define SA_NOCLDSTOP	0x00000004
#define SA_NODEFER	0x00000008
#define SA_RESETHAND	0x00000010
#define SA_NOCLDWAIT	0x00000020 /* not supported yet */
#define SA_SIGINFO	0x00000040
#define SA_INTERRUPT	0x20000000 /* dummy -- ignored */
#elif defined(__hppa__)
#define SA_ONSTACK	0x00000001
#define SA_RESETHAND	0x00000004
#define SA_NOCLDSTOP	0x00000008
#define SA_SIGINFO	0x00000010
#define SA_NODEFER	0x00000020
#define SA_RESTART	0x00000040
#define SA_NOCLDWAIT	0x00000080 /* not supported yet */
#define _SA_SIGGFAULT	0x00000100 /* HPUX */
#define SA_INTERRUPT	0x20000000 /* dummy -- ignored */
#define SA_RESTORER	0x04000000 /* obsolete -- ignored */
#elif defined (__sparc__)
#define SV_SSTACK	1	/* This signal handler should use sig-stack */
#define SV_INTR		2	/* Sig return should not restart system call */
#define SV_RESET	4	/* Set handler to SIG_DFL upon taken signal */
#define SV_IGNCHILD	8	/* Do not send SIGCHLD */

#define SA_NOCLDSTOP	SV_IGNCHILD
#define SA_STACK	SV_SSTACK
#define SA_ONSTACK	SV_SSTACK
#define SA_RESTART	SV_INTR
#define SA_RESETHAND	SV_RESET
#define SA_INTERRUPT	0x10
#define SA_NODEFER	0x20
#define SA_SHIRQ	0x40
#define SA_NOCLDWAIT	0x100	/* not supported yet */
#define SA_SIGINFO	0x200
#else
#if defined (__mips__)
#define SA_NOCLDSTOP	0x00000001
#define SA_SIGINFO	0x00000008
#define SA_NOCLDWAIT	0x00010000 /* Not supported yet */
#else
#define SA_NOCLDSTOP	0x00000001
#define SA_NOCLDWAIT	0x00000002 /* not supported yet */
#define SA_SIGINFO	0x00000004
#endif
#if defined(__arm__)
#define SA_THIRTYTWO	0x02000000
#endif
#define SA_RESTORER	0x04000000
#define SA_ONSTACK	0x08000000
#define SA_RESTART	0x10000000
#define SA_INTERRUPT	0x20000000 /* dummy -- ignored */
#define SA_NODEFER	0x40000000
#define SA_RESETHAND	0x80000000
#endif

/* ugh, historic Linux legacy, for gpm :-( */
#define SA_NOMASK	SA_NODEFER
#define SA_ONESHOT	SA_RESETHAND

/* sigaltstack controls */
#define SS_ONSTACK	1
#define SS_DISABLE	2

#define MINSIGSTKSZ	2048
#define SIGSTKSZ	8192

#define SIG_DFL ((sighandler_t)0L)	/* default signal handling */
#define SIG_IGN ((sighandler_t)1L)	/* ignore signal */
#define SIG_ERR ((sighandler_t)-1L)	/* error return from signal */

typedef void (*sighandler_t)(int);
typedef struct siginfo siginfo_t;


typedef struct {
  unsigned long sig[_NSIG_WORDS];
} sigset_t;

struct sigaction {
#if defined(__alpha__) || defined(__ia64__) || defined(__hppa__)
  union {
    sighandler_t _sa_handler;
    void (*_sa_sigaction)(int, siginfo_t*, void*);
  } _u;
  unsigned long sa_flags;
  sigset_t sa_mask;
#elif defined(__mips__)
  unsigned long sa_flags;
  union {
    sighandler_t _sa_handler;
    void (*_sa_sigaction)(int, siginfo_t*, void*);
  } _u;
  sigset_t sa_mask;
  void (*sa_restorer)(void);
  int32_t sa_resv[1];
#else	/* arm, i386, ppc, s390, sparc, saprc64, x86_64 */
  union {
    sighandler_t _sa_handler;
    void (*_sa_sigaction)(int, siginfo_t*, void*);
  } _u;
  unsigned long sa_flags;
  void (*sa_restorer)(void);
  sigset_t sa_mask;
#endif
};

#define sa_handler	_u._sa_handler
#define sa_sigaction	_u._sa_sigaction

#if defined(__sparc__)
#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	4
#elif defined(__alpha__) || defined(__mips__)
#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	3
#else
#define SIG_BLOCK	0	/* for blocking signals */
#define SIG_UNBLOCK	1	/* for unblocking signals */
#define SIG_SETMASK	2	/* for setting the signal mask */
#endif

int kill(pid_t pid, int sig);
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signo);
int sigsuspend(const sigset_t *mask);
int sigpending(sigset_t *set);
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

#endif
