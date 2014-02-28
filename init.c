#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

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

/* default/built-in stuff */
const char* inittab = INITTAB;

/* configuration */
weak struct config* cfg;

/* misc */
int initctlfd;		// listening socket
sigset_t defsigset;	// default sigset, to supply to spawned processes,
			// and also to use outside of ppoll in init itself

/* --------------------------------------------------------------------------- */
static int setup(int argc, char** argv);
static int setinitctl(void);
static void setsignals(void);
static void setargs(int argc, char** argv);

extern int configure(int);
extern void setnewconf(void);

extern void initpass(void);
extern void pollfds(void);
extern void acceptctl(void);
extern void waitpids(void);

static void sighandler(int sig);

/* --------------------------------------------------------------------------- */

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

		/* initpass finished without any pending w/o-type processes,
		   so it's ok to change configuration */
		if((state & S_RECONFIG) && !(state & S_WAITING))
			setnewconf();

		if(!nextlevel && !(state & S_WAITING))
			goto reboot;

		/* Block for at most $waitneeded, waiting for signals
		   or telinit commands. Only set state flags here, do
		   not do any processing. */
		pollfds();

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
	warn("still here, reboot(0x%08X) failed: %m", rbcode);

	return 0xFE; /* feh */
};

static int setup(int argc, char** argv)
{
	currlevel = 0;
	nextlevel = INITDEFAULT;
	rbcode = LINUX_REBOOT_CMD_HALT;
	syslogfd = -1;

	if(getpid() == 1)
		state |= S_PID1;

	ioctl(0, KDSIGACCEPT, SIGWINCH);

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

/* init gets the whole kernel command line, "root=... rw initrd=... console=..." etc.
   The only relevant part there is possible initial runlevel indication, either
   a (single-digit) number or a word "single". */
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
   of normal operation range and should accept whatever default action is. */
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
	   After all, BSD-compatible signal() implementation (which is to say,
	   pretty much all of them) are just wrappers around sigaction(2). */

	/* For the sake of clarity, avoid mixing handled signal */
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
	initctlfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
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
			
		/* mostly useless with S_PID1, but makes life easier when testing */
		case SIGTERM:
		case SIGINT:
			nextlevel = 0;
			break;

		case SIGHUP:
			if(initctlfd >= 0)
				close(initctlfd);
			setinitctl();
			break;
	}
}
