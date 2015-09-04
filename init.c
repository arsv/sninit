#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"
#include "sys.h"
#include "init.h"
#include "scope.h"

/* Init compiles inittab (and/or initdir) into struct config and uses
   it to keep all per-process data. The struct is essentially static,
   in that its size does not change once it has been compiled.

   When reconfiguring, a new struct is allocated, relevant data is copied
   there, then this pointer gets switched to the new struct.

   The struct is declared weak to allow linking build-in inittab over. */

weak struct config* cfg;

/* Sninit uses the notion of runlevels to tell which entries from inittab
   should be running at any given moment and which should not.

   At any given time, init "is in" a single primary runlevel, possibly
   augmented with any number of sublevels. The whole thing is stored as
   a bitmask: runlevel 3ab is (1<<3) | (1<<a) | (1<<b).

   Switching between runlevels is initiated by setting nextlevel to something
   other than currlevel. The switch is completed once currlevel = nextlevel.

   Check shouldberunning() on how entries are matched against current
   runlevel, and initpass() for level-switching code.

   Init starts at runlevel 0, so 0~ entries are not spawned during boot.
   When shutting down, we first switch back to level 0 = (1<<0) and then
   to "no-level" which is value 0, making sure all entries get killed. */

int currlevel = (1 << 0);
int nextlevel = INITDEFAULT;

/* Normally init sleeps in ppoll until dusturbed by a signal or a socket
   activity. However, initpass may want to set an alarm, so that it would
   send SIGKILL 5 seconds after SIGTERM if the process refuses to die.
   This is done by setting timetowait, which is later used for ppoll timeout.

   Default value here is -1, which means "sleep indefinitely".
   main sets that before each initpass() */

int timetowait;

/* To set timestamps on initrecs (lastrun, lastsig), initpass must have
   some kind of current time value available. Instead of making a syscall
   for every initrec that needs it, the call is only made before initpass
   and the same value is then used during the pass. See also setpasstime() */

time_t passtime;

/* sninit shuts down the system by calling reboot(rbcode) after exiting
   the main loop. In other words, reboot command internally is just
       nextlevel = (1<<0);
       rbcode = RB_AUTOBOOT;
   The values are described in <sys/reboot.h> (or check reboot(2)) */

int rbcode = RB_HALT_SYSTEM;

/* These fds are kept open more or less all the time.
   initctl is the listening socket, syslogfd is /dev/log, warnfd is
   where warn() will put its messages. */

int initctlfd;
int syslogfd = -1;	/* not yet opened */
int warnfd = 0;		/* stderr only, see warn() */

/* Init blocks all most signals when not in ppoll. This is the orignal
   pre-block signal mask, used for ppoll and passed to spawned children. */

sigset_t defsigset;

/* S_* flags to signal we have a pending telinit connection or
   a reconfiguration request */

int state = 0;

/* Short outline of the code: */

export int main(int argc, char** argv);		/* main loop */

local int setup(int argc, char** argv);		/* initialization */
local int setinitctl(void);
local void setsignals(void);
local void setargs(int argc, char** argv);
local int setpasstime(void);

extern int configure(int);		/* inittab parsing */
extern void setnewconf(void);

extern void initpass(void);		/* the core: process (re)spawning */
extern void waitpids(void);

extern void pollctl(void);		/* telinit communication */
extern void acceptctl(void);

local void sighandler(int sig);		/* global singnal handler */
local void forkreboot(void);		/* reboot(rbcode) */


/* main(), the entry point also the main loop.

   Overall logic here: interate over inittab records (that's initpass()),
   go sleep in ppoll(), iterate, sleep in ppoll, iterate, sleep in ppoll, ...

   Within this cycle, ppoll is the only place where blocking occurs.
   Even when running a wait entry, init does not use blocking waitpid().
   Instead, it spawns the process and goes to sleep in ppoll until
   the process dies.

   Whenever there's a need to disturb the cycle, flags are raised in $state.
   Any branching to handle particular situation, like child dying or telinit
   knocking on the socket, occurs here in main.

   For time-tracking code, see longish comment near setpasstime() below.
   Time is only checked once for each initpass (that's why "passtime").

   Barring early hard errors, the only way to exit the main loop is switching
   to "no-runlevel" state, which is (currlevel == 0). Note this is different
   from runlevel 0 which is (1<<0). See the block at the end of initpass. */

int main(int argc, char** argv)
{
	if(setup(argc, argv))
		goto reboot;	/* Initial setup failed badly */
	if(setpasstime())
		passtime = BOOTCLOCKOFFSET;

	while(1)
	{
		warnfd = 2;
		timetowait = -1;

		initpass();		/* spawn/kill processes */

		if(!currlevel)
			goto reboot;

		if(!(state & S_WAITING) && (state & S_RECONFIG))
			setnewconf();

		pollctl();		/* waiting happens here */

		if(setpasstime() && timetowait > 0)
			passtime += timetowait;

		if(state & S_SIGCHLD)
			waitpids();	/* reap dead children */

		if(state & S_INITCTL)
			acceptctl();	/* telinit communication */
	}

reboot:
	warnfd = 0;		/* stderr only, do not try syslog */
	if(!(state & S_PID1))	/* we're not running as *the* init */
		return 0;

	forkreboot();
	return 0xFE; /* feh */
};

/* During startup no user interaction is possible, so init must somehow
   cope with what it has got, or just bail out.

   When inittab is read for the first time, most errors are ignored,
   and incorrect entries are dropped. The idea is that incomplete
   configuration may still be enough for basic ui to come up, allowing
   the user to fix whatever is wrong.

   In case inittab file is missing, we try to fall back to built-in config.

   Init starts by calling getpid(), partially to know whether it is ok to
   call reboot() later, and partially to test environment sanity.
   getpid() can not fail, that is, unless it fails with ENOSYS, in which case
   we are probably running x86{,_32,_64} with an incompatible kernel. */

int setup(int argc, char** argv)
{
	if(getpid() == 1)
		state |= S_PID1;
	else if(errno)
		_exit(errno);

	if(setinitctl())
		/* Not having telinit is bad, but aborting system startup
		   for this mere reason is likely even worse. */
		warn("can't initialize initctl, init will be uncontrollable");

	setsignals();
	setargs(argc, argv);

	if(!configure(NONSTRICT))
		setnewconf();
	else if(!cfg)
		retwarn(-1, "initial configuration error");

	return 0;
}

/* Init gets any part of kernel command line the kernel itself could not
   parse. Among those, the only thing that concerns init is possible initial
   runlevel indication, either a (single-digit) number or a word "single".

   Init does not pass its argv to any of the children. */

void setargs(int argc, char** argv)
{
	char** argi;

	for(argi = argv; argi - argv < argc; argi++)
		if(!strcmp(*argi, "single"))
			nextlevel = (1 << 1);
		else if(**argi >= '1' && **argi <= '9' && !*(*argi+1))
			nextlevel = (1 << (**argi - '0'));
}

/* Outside of ppoll, we only block SIGCHLD; inside ppoll, default sigmask
   is used. This should be ok since linux blocks signals to init from other
   processes, and blocking kernel-generated signals rarely makes sense.
   Normally init shouldn't be getting them, aside from SIGCHLD and maybe
   SIGPIPE/SIGALARM during telinit communication. If anything else is sent
   (SIGSEGV?), then we're already well out of normal operation range
   and should accept whatever the default action is.

   SIGPIPE and SIGALRM do not need handlers, as their only job is to make
   blocking read(telinitfd) return with EINTR, which telinit code interprets
   as end-of-communication.
   SIGCHLD must interrupt the only syscall it may be delivered in, ppoll.
   All the other signals need SA_RESTART. */

void setsignals(void)
{
	struct sigaction sa = {
		.sa_handler = sighandler,
		.sa_flags = SA_RESTART,
	};

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &defsigset);

	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGHUP);

	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP,  &sa, NULL);

	/* SIGCHLD is only allowed to arrive in ppoll,
	   so SA_RESTART just does not make sense. */
	sa.sa_flags = 0;
	sigaction(SIGCHLD, &sa, NULL);

	/* These *should* interrupt write() calls,
	   which is the opposite of SA_RESTART. */
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGALRM, &sa, NULL);
}

int setinitctl(void)
{
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = INITCTL
	};

	/* This way readable "@initctl" can be used for reporting below,
	   and config.h looks better too. */
	if(addr.sun_path[0] == '@')
		addr.sun_path[0] = '\0';

	/* we're not going to block for connections, just accept whatever
	   is already there; so it's SOCK_NONBLOCK */
	if((initctlfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0)) < 0)
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

/* A single handler for all four signals we care about.
   SIGALARM is not handled, as its only function is to make
   write() return EINTR. */

void sighandler(int sig)
{
	switch(sig)
	{
		case SIGCHLD:
			state |= S_SIGCHLD;
			break;

		case SIGTERM:	/* C-c when testing */
		case SIGINT:	/* C-A-Del with S_PID1 */
			rbcode = RB_AUTOBOOT;
			nextlevel = (1<<0);
			break;

		case SIGHUP:
			if(initctlfd >= 0)
				close(initctlfd);
			setinitctl();
			break;
	}
}

/* Throughout the loop, main() keeps track of current time which initpass()
   then uses for things like timed SIGTERM/SIGKILL, fast respawns and so on.
   Via timetowait, the above affects ppoll timeout in pollfds(),
   making the main loop run a bit faster than it would with only SIGCHLDs
   and telinit socket noise.

   Precision is not important here, but keeping sane timeouts is crucial;
   setting poll timeout to 0 consistently by mistake would result in the loop
   spinning out of control. To counteract this, setpasstime errors are handled
   by pushing passtime forward, pretending ppoll call never returns early.

   Note clock errors are not something that happens daily, and usually
   is's a sign of deep troubles, like running on an incompatible architecture.
   Still, once we have the system running, it makes sense to try handling
   the situation gracefully. After all, timing stuff is somewhat auxillilary
   in a non-realtime unix, a matter of convenience, not correctness, and
   init could (should?) have been written with no reliance on time. */

/* The value used for passtime is kernel monotonic clock shifted
   by a constant. The code only uses passtime differences, not the value itself.
   Constant shift is necessary to make sure the difference is not zero
   at the first initpass, to avoid triggering time_to_* stuff.

   The offset may be as low as the actual value of time_to_restart,
   but since time_to_restart is a short using 0xFFFF is a viable option.
   After all, even with offset that large monotonic clock values are much
   much lower than those routinely returned by CLOCK_REALTIME. */

int setpasstime(void)
{
	struct timespec tp;

	if(clock_gettime(CLOCK_MONOTONIC, &tp))
		retwarn(-1, "clock failed: %m");

	passtime = tp.tv_sec + BOOTCLOCKOFFSET;

	return 0;
}

/* Linux kernel treats reboot() a lot like _exit(), including, quite
   surprisingly, panic when it's init who calls it. So we've got to fork
   here and call reboot for the child process. Now because vfork may
   in fact be fork, and scheduler may still be doing tricks, it's better
   to wait for the child before returning, because return here means
   _exit from init and immediate panic. */

void forkreboot(void)
{
	int pid;
	int status;

	if((pid = vfork()) == 0)
		_exit(reboot(rbcode));
	else if(pid < 0)
		return;
	if(waitpid(pid, &status, 0) < 0)
		return;
	if(!WIFEXITED(status) || WEXITSTATUS(status))
		warn("still here, reboot failed, time to panic");
}
