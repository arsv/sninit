#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/wait.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "config.h"
#include "init.h"

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

   Init starts at runlevel 0, so X0 entries are not spawned during boot.
   When shutting down, we first switch back to level 0 = (1<<0) and then
   to "no-level" which is value 0, making sure all entries get killed. */

int currlevel = (1 << 0);
int nextlevel = INITDEFAULT;

/* Normally init sleeps in ppoll until dusturbed by a signal or a socket
   activity. However, initpass may want to set an alarm, so that it would
   send SIGKILL 5 seconds after SIGTERM if the process refuses to die.
   This is done by setting timetowait, which is later used for ppoll timeout.

   Default value here is -1, which means "sleep indefinitely".
   main resets the timer before each initpass(). */

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
   initctl is the listening socket, warnfd is where warn()
   will put its messages.
   There is also syslogfd which is kept and managed in init_warn. */

extern int initctlfd;
extern int warnfd;

/* Init blocks most signals when not in ppoll. This is the orignal pre-block
   signal mask, used for ppoll and passed to spawned children. */

sigset_t defsigset;

/* S_* flags to signal we have a pending telinit connection or
   a reconfiguration request */

int state = 0;

/* Init gets any part of kernel command line the kernel itself could not
   parse. Among those, the only thing that concerns init is possible initial
   runlevel indication, either a (single-digit) number or a word "single".

   Unlike sysvinit, sninit does not pass its argv to any of its children. */

static void setargs(int argc, char** argv)
{
	char** argi;

	for(argi = argv; argi - argv < argc; argi++)
		if(!strcmp(*argi, "single"))
			nextlevel = (1 << 1);
		else if(**argi >= '1' && **argi <= '9' && !*(*argi+1))
			nextlevel = (1 << (**argi - '0'));
}

/* Most applications (sninit included) expect fds 0, 1, 2 to be open.
   Kernel tries to fulfill that, opening and duping /dev/console just
   before spawning init. However it does not check the results, and opening
   /dev/console can fail, in particular if there is no console configured.

   In case this happens, unused fds remain available and may be reused later
   by regular open() calls, potentially leading to really nasty situations
   with stuff written to stderr ending up in unrelated files.

   To avoid that, the code below makes sure fds 0-2 are open and safe
   for writing.

   See kernel/init/main.c kernel_init(), kernel_init_freeable() and also
   busybox/init/init.c console_init() around bb_sanitize_stdio() following to
   bb_daemonize_or_rexec() from libb/vfork_daemon_rexec.c. */

static int setstdfds(void)
{
	int fd;

	if(fcntl(2, F_GETFD) >= 0)
		return 0; /* if 2 is ok, then 0 and 1 must be valid as well */

	if((fd = open("/dev/null", O_RDWR)) >= 0)
		goto gotfd;
	if((fd = open("/", O_RDONLY)) >= 0)
		goto gotfd;
	/* Not being able to open / read-only is weird enough to panic */
	return -1;

gotfd:
	if(fd < 1)
		dup2(fd, 1);
	if(fd < 2)
		dup2(fd, 2);
	if(fd > 2)
		close(fd);
	return 0;
}

/* A single handler for all signals we care about. */

static void sighandler(int sig)
{
	switch(sig)
	{
		case SIGCHLD:
			state |= S_SIGCHLD;
			break;

		case SIGTERM:	/* C-c when testing */
		case SIGINT:	/* C-A-Del */
			rbcode = RB_AUTOBOOT;
			nextlevel = (1<<0);
			break;

		case SIGPWR:	/* Shutdown request */
			rbcode = RB_POWER_OFF;
			nextlevel = (1<<0);
			break;

		case SIGHUP:
			/* close the socket here but defer reopening,
			   that's way too much to do in singhandler */
			if(initctlfd >= 0)
				close(initctlfd);
			initctlfd = -1;
			state |= S_REOPEN;
			break;

		/* SIGPIPE and SIGALRM need no handling, as their only job
		   is to make blocking read(telinitfd) return with EINTR,
		   which initctl code interprets as end-of-communication. */
	}
}

/* Outside of ppoll, we only block SIGCHLD; inside ppoll, default sigmask
   is used. This should be ok since linux blocks signals to init from other
   processes, and blocking kernel-generated signals rarely makes sense.

   SIGCHLD must interrupt the only syscall it may be delivered in, ppoll. */

static int setsignals(void)
{
	struct sigaction sa = {
		.sa_handler = sighandler,
		.sa_flags = SA_RESTART,
	};
	/* The stuff below *can* fail due to broken libc, but that is so bad
	   by itself that there is no point in reporting it properly. */
	int ret = 0;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGCHLD);
	ret |= sigprocmask(SIG_BLOCK, &sa.sa_mask, &defsigset);

	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGPWR);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGHUP);

	ret |= sigaction(SIGINT,  &sa, NULL);
	ret |= sigaction(SIGPWR,  &sa, NULL);
	ret |= sigaction(SIGTERM, &sa, NULL);
	ret |= sigaction(SIGHUP,  &sa, NULL);

	/* SIGCHLD is only allowed to arrive in ppoll,
	   so SA_RESTART just does not make sense. */
	sa.sa_flags = 0;
	ret |= sigaction(SIGCHLD, &sa, NULL);

	/* These *should* interrupt write() calls, which is the opposite
	   of SA_RESTART. There is no handler code for these, but SIG_IGN
	   prevents syscall interruption, so we have to leave &sighandler
	   in sa_handler. */
	ret |= sigaction(SIGPIPE, &sa, NULL);
	ret |= sigaction(SIGALRM, &sa, NULL);

	return ret;
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

static int setpasstime(void)
{
	struct timespec tp;

	if(clock_gettime(CLOCK_MONOTONIC, &tp))
		retwarn(-1, "clock failed: %m");

	passtime = tp.tv_sec + BOOTCLOCKOFFSET;

	return 0;
}

/* Init spends most of its time in ppoll() here, waiting for signals
   or incoming telinit requests.

   This is also the only place where handled signals (including SIGCHLD)
   may arrive. Outside of ppoll, only non SIGPIPE and SIGALRM are allowed,
   and only because their purpose is to interrupt stuck read()s write()s. */

static void pollctl(void)
{
	int r;
	struct pollfd pfd;
	struct timespec pts;
	struct timespec* ppts;

	pfd.fd = initctlfd;
	pfd.events = POLLIN;
	if(timetowait >= 0) {
		pts.tv_sec = timetowait;
		pts.tv_nsec = 0;
		ppts = &pts;
	} else {
		ppts = NULL;
	}

	r = ppoll(&pfd, 1, ppts, &defsigset);

	if(r < 0 && errno != EINTR) {
		/* Failed ppoll means the main loop becomes unconstrained,
		   making init uncontrollable and wasting cpu cycles.
		   To avoid that, let's try to slow things down a bit. */
		warn("poll failed: %m");

		sigset_t cursigset;
		pts.tv_sec = timetowait >= 0 ? timetowait : 1;
		pts.tv_nsec = 0;

		/* ppoll also handles sigmask-lifting, try to work around that */
		sigprocmask(SIG_SETMASK, &defsigset, &cursigset);
		nanosleep(&pts, NULL);
		sigprocmask(SIG_SETMASK, &cursigset, NULL);

		/* EINTR, on the other hand, is totally ok (SIGCHLD etc) */
	} else if(r > 0) {
		/* only one fd in pfd, so not that much choice here */
		state |= S_INITCTL;
	}
}

/* Linux kernel treats reboot() a lot like _exit(), including panic()
   when it's init who calls it. To prevent panic, reboot() is called
   in its own process.

   Any return here means _exit from init and immediate panic.

   Both the parent and the child return, expecting the caller
   to proceed to _exit immediately. */

#ifdef NOMMU
#define fork() vfork()
#endif

static int forkreboot(void)
{
	int pid;
	int status;

	if((pid = fork()) == 0)
		return reboot(rbcode);
	else if(pid > 0)
		waitpid(pid, &status, 0);

	/* By this point, it does not really matter what exactly failed,
	   fork() or wait() or reboot() */

	warn("still here, reboot failed, time to panic");
	return 0xFE;
}

/* During startup no user interaction is possible, so init has to cope
   somehow with what it has got.

   When inittab is read for the first time, most errors are ignored,
   and incorrect entries are dropped. The idea is that incomplete
   configuration may still be enough for basic ui to come up, allowing
   the user to fix whatever is wrong.

   In case inittab file is missing, we try to fall back to built-in config. */

static int setup(int argc, char** argv)
{
	setargs(argc, argv);

	if(setstdfds())
		retwarn(-1, "cannot set initial fds 0, 1, 2");

	if(setinitctl())
		/* Not having telinit is bad, but aborting system startup
		   for this mere reason is likely even worse. */
		warn("can't initialize initctl, init will be uncontrollable");

	if(setsignals())
		retwarn(-1, "failed to set signal handlers");

	if(!configure(NONSTRICT))
		setnewconf();
	else if(!cfg)
		retwarn(-1, "initial configuration error");

	return 0;
}

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

		initpass();	/* spawn/kill processes */

		if(!currlevel)
			goto reboot;

		pollctl();	/* waiting happens here */

		if(setpasstime() && timetowait > 0)
			passtime += timetowait;

		if(state & S_SIGCHLD)
			waitpids();
		if(state & S_INITCTL)
			acceptctl();
		if(state & S_REOPEN)
			setinitctl();
		state &= ~(S_SIGCHLD | S_INITCTL | S_REOPEN);

		if(currlevel != nextlevel)
			continue;

		if(state & S_RECONFIG)
			setnewconf();
		state &= ~S_RECONFIG;
	}

reboot:
	warnfd = 0;		/* stderr only, do not try syslog */

	if(getpid() != 1)	/* not running as *the* init */
		return 0;

	return forkreboot();
};
