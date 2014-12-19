#ifndef BITS_SIGNAL_H
#define BITS_SIGNAL_H

#define _NSIG		128
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
#define SIGEMT		 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGBUS		10
#define SIGSEGV		11
#define SIGSYS		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
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

#define SIGCLD		SIGCHLD
#define SIGPOLL		SIGIO

#define SIGLOST		SIGPWR
#define SIGRTMIN	32
#define SIGRTMAX	(_NSIG-1)

/* SA_FLAGS values: */
#define SA_NOCLDSTOP	0x00000001
#define SA_SIGINFO	0x00000008
#define SA_NOCLDWAIT	0x00010000 /* Not supported yet */
#define SA_RESTORER	0x04000000
#define SA_ONSTACK	0x08000000
#define SA_RESTART	0x10000000
#define SA_INTERRUPT	0x20000000 /* dummy -- ignored */
#define SA_NODEFER	0x40000000
#define SA_RESETHAND	0x80000000

/* sigaltstack controls */
#define SS_ONSTACK	1
#define SS_DISABLE	2

#define MINSIGSTKSZ	2048
#define SIGSTKSZ	8192

/* MIPS-specific */
#define SIG_BLOCK	1	/* for blocking signals */
#define SIG_UNBLOCK	2	/* for unblocking signals */
#define SIG_SETMASK	3	/* for setting the signal mask */

#define SIG_DFL ((sighandler_t)0L)	/* default signal handling */
#define SIG_IGN ((sighandler_t)1L)	/* ignore signal */
#define SIG_ERR ((sighandler_t)-1L)	/* error return from signal */

typedef void (*sighandler_t)(int);
typedef struct siginfo siginfo_t;

typedef struct {
	unsigned long sig[_NSIG_WORDS];
} sigset_t;

struct sigaction {
	unsigned long sa_flags;
	union {
		sighandler_t _sa_handler;
		void (*_sa_sigaction)(int, siginfo_t*, void*);
	} _u;
	sigset_t sa_mask;
	void (*sa_restorer)(void);
	int32_t sa_resv[1];
};

#define sa_handler	_u._sa_handler
#define sa_sigaction	_u._sa_sigaction

#endif
