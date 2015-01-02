#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"
#include "sys.h"
#include "init.h"

/* status */
int currlevel;		// currently occupied runlevel bitmask
int nextlevel;		// the one we're switching to; =curlevel when we're done switching
int state;		// S_* flags
int timetowait;		// poll timeout, ms. Cleared in main(), set by waitneeded(), checked by pollfds()
int warnfd;		// primary log fd (see init_warn.c)
int rbcode;		// reboot code, for reboot(2)

int syslogfd;		// syslog; see init_warn.c

/* configuration */
weak struct config* cfg;

/* misc */
int initctlfd;		// listening socket
sigset_t defsigset;	// default sigset, to supply to spawned processes,
			// and also to use outside of ppoll in init itself

static int setup(int argc, char** argv);
static int setinitctl(void);
static void setsignals(void);
static void setargs(int argc, char** argv);

extern int configure(int);
extern void setnewconf(void);

extern void initpass(void);
extern void pollctl(void);
extern void acceptctl(void);
extern void waitpids(void);

static void sighandler(int sig);

/* Overall logic in main: interate over inittab records (that's initpass()),
   go sleep in ppoll(), iterate, sleep in ppoll, iterate, sleep in ppoll, ...

   Within this cycle, ppoll is the only place where blocking occurs.
   Even when running :wait: line, init does not use blocking waitpid().
   Instead, it spawns the process and goes to sleep in ppoll until
   the process dies.

   Whenever there's a need to disturb the cycle, flags are raised in $state.
   Any branching to handle particular situation, like child dying or telinit
   knocking on the socket, occurs here in main.  */

int main(int argc, char** argv)
{
	if(setup(argc, argv))
		goto reboot;	/* Initial setup failed badly */

	while(1)
	{
		warnfd = 2;
		timetowait = -1;

		/* (Re)spawn processes that need (re)spawning */
		initpass();

		/* "No runlevel at all". This is the state after reaching runlevel 0
		   which is (1<<0). See initpass for explaination. */
		if(!currlevel)
			goto reboot;

		/* initpass finished without any pending w/o-type processes,
		   so it's ok to change configuration */
		if(!(state & S_WAITING) && (state & S_RECONFIG))
			setnewconf();

		/* Block for at most $waitneeded, waiting for signals
		   or (if the socket is open) telinit commands.
		   Only set state flags here, do not do any processing. */
		pollctl();

		/* reap dead children */
		if(state & S_SIGCHLD)
			waitpids();

		/* check for telinit commands, if any */
		if(state & S_INITCTL)
			acceptctl();
	}

reboot:
	warnfd = 0;		/* stderr only, do not try syslog */
	if(!(state & S_PID1))	/* we're not running as *the* init, just exit quietly */
		return 0;

	reboot(rbcode);
	warn("still here, reboot(%i) failed: %m", rbcode);

	return 0xFE; /* feh */
};

static int setup(int argc, char** argv)
{
	/* Runlevel 0. This is necessary to make sure :~0:wait: type entries
	   get marked as "has been run" upon initialization. */
	currlevel = 1 << 0;
	nextlevel = INITDEFAULT;
	rbcode = RB_HALT_SYSTEM;
	syslogfd = -1;

	/* To avoid calling getpid every time. And since this happens to be
	   the first syscall init makes, it is also used to check whether runtime
	   situation is bearable. */
	if(getpid() == 1)
		state |= S_PID1;
	/* Failing getpid() is a sign of big big trouble, like running x86 or x32
	   on a x64 kernel without relevant parts built in. If it is the case,
	   error reporting is pointless and all init can do is bail out asap. */
	else if(errno)
		_exit(errno);

	if(setinitctl())
		/* Not having telinit is bad, but aborting system startup
		   for this mere reason is likely even worse. */
		warn("can't initialize initctl, init will be uncontrollable");

	setsignals();
	setargs(argc, argv);

	if(!configure(0))
		setnewconf();
	else if(!cfg)
		retwarn(-1, "initial configuration error");

	return 0;
}

/* init gets any part of kernel command line the kernel itself could not parse.
   Among those, the only thing that concerns init is possible initial runlevel
   indication, either a (single-digit) number or a word "single".
   init does not pass its argv to any of the children. */
static void setargs(int argc, char** argv)
{
	char** argi;

	for(argi = argv; argi - argv < argc; argi++)
		if(!strcmp(*argi, "single"))
			nextlevel = (1 << 1);
		else if(**argi >= '1' && **argi <= '9' && !*(*argi+1))
			nextlevel = (1 << (**argi - '0'));
}

/* Outside of ppoll, we only block SIGCHLD; inside ppoll, default sigmask is used.
   This should be ok since linux blocks signals to init from other processes, and
   blocking kernel-generated signals rarely makes sense. Normally init shouldn't be
   getting them, aside from SIGCHLD and maybe SIGPIPE/SIGALARM during telinit
   communication. If anything else is sent (SIGSEGV?), then we're already well out
   of normal operation range and should accept whatever the default action is. */
static void setsignals(void)
{
	/* Restarting read() etc is ok, the calls init needs interrupted
	   will be interrupted anyway.
	   Exception: SIGALRM must be able to interrput write(), telinit
	   timeout handling is built around this fact. */
	struct sigaction sa = {
		.sa_handler = sighandler,
		.sa_flags = SA_RESTART,
	};

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &defsigset);

	/* These should have been signal(2) calls, but since signal(2) tells us
	   to "avoid its use", we'll call sigaction instead.
	   After all, BSD-compatible signal() implementations (which is to say,
	   pretty much all of them) are just wrappers around sigaction(2). */

	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGHUP);

	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP,  &sa, NULL);

	sa.sa_flags = SA_NOCLDSTOP; 	/* init does not care about children being stopped */
	sigaction(SIGCHLD, &sa, NULL);
	
	/* These should interrupt write() calls, and that's enough */
	sa.sa_flags = 0;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
}

static int setinitctl(void)
{
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = INITCTL
	};

	if(addr.sun_path[0] == '@')
		addr.sun_path[0] = '\0';

	/* we're not going to block for connections, just accept whatever
	   is already there; so it's SOCK_NONBLOCK */
	if((initctlfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0)
		retwarn(-1, "Can't create control socket: %m");

	if(bind(initctlfd, (struct sockaddr*)&addr, sizeof(addr))) 
		gotowarn(close, "Can't bind %s: %m", INITCTL)
	else if(listen(initctlfd, 1))
		gotowarn(close, "listen() failed: %m");

	return 0;

close:
	close(initctlfd);
	initctlfd = -1;
	return -1;
}

/* A single handler for all four signals we care about. */
static void sighandler(int sig)
{
	switch(sig)
	{
		case SIGCHLD:
			state |= S_SIGCHLD;
			break;
			
		case SIGTERM:	/* C-c when testing */
		case SIGINT:	/* C-A-Del with S_PID1 */
			nextlevel = (1<<0);
			break;

		case SIGHUP:
			if(initctlfd >= 0)
				close(initctlfd);
			setinitctl();
			break;
	}
}
