#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "init.h"

/* bits for waitfor */
#define DYING		(1<<0)
#define RUNNING		(1<<1)

extern int state;
extern int currlevel;
extern int nextlevel;
extern int timetowait;
extern int initctlfd;
extern struct config* cfg;

extern void execinitrec(struct initrec* p);

static void spawn(struct initrec* p);
global void stop(struct initrec* p);

static time_t monotime(void);
static inline void swapi(int* a, int* b);
static inline int shouldberunning(struct initrec* p);

/* Initpass: go through inittab, (re)starting entries
   that need to be (re)started and killing entries that should be killed.

   There are two principal passes over inittab: bottom-to-top that kills
   processes first, and top-to-bottom that spawns processes. Backwards
   pass is needed to C_WAIT without C_ONCE, i.e. waiting for things to
   die before killing certain s-type records (syslog primarily).

   In case a C_WAIT entry is reached during either pass, relevant action
   is performed and initpass return.
   SIGCHLD will arrive on entry completition, triggering another initpass.
   Blocking wait is never used, init waits for w-type entries in ppoll().
 
   Because no explicit list pointer is kept during runlevel switching,
   things get a bit tricky with w-/o-type entries which are traversed
   several times but should only be run once.
   Note to have pid reset to 0, and thus allow re-run, at least one pass
   must be completed with !shouldberunning(p) for the entry. */

#define wtype(p) ((p->flags & (C_ONCE | C_WAIT)) == (C_ONCE | C_WAIT))
#define htype(p) ((p->flags & (C_ONCE | C_WAIT)) == (C_ONCE))
#define owtype(p) ((p->flags & C_ONCE))

void initpass(void)
{
	int waitfor = 0;
	struct initrec** pp;
	struct initrec* p;

	if(!cfg->inittab || cfg->initnum <= 0)
		return;	/* should never happen, but who knows */

	struct initrec** inittab = cfg->inittab;
	struct initrec** initend = cfg->inittab + cfg->initnum - 1;

	state |= S_WAITING;

	/* Kill pass, reverse order */
	for(pp = initend; (p = *pp); pp--)
		if(!shouldberunning(p) && p->pid > 0)
		{
			stop(p);

			waitfor |= DYING;

			if(htype(p))
				return;
		}

	/* Run pass, direct order */
	for(pp = inittab; (p = *pp); pp++)
		if(shouldberunning(p))
		{
			if(p->pid > 0) {
				if(wtype(p))
					return;
				else if(owtype(p))
					waitfor |= RUNNING;
				continue;
			}

			if(p->pid < 0 && owtype(p))
				continue; /* has been run already */
			if(waitfor && wtype(p))
				return;

			spawn(p);

			if(owtype(p))
				waitfor |= RUNNING;	/* we're not in nextlevel yet */
			if(wtype(p))
				return;			/* off to wait for this process */
		}

	if(!waitfor)
		state &= ~S_WAITING;
 	if(waitfor || currlevel == nextlevel)
		return;

	/* Nothing more to run, we've done switching runlvls */

	/* One we're here, reset pid for o-type entries, to run them when
	   entering another runlevel with shouldberunning(p) true. */
	for(pp = inittab; (p = *pp); pp++)
		if(!shouldberunning(p) && owtype(p) && (p->pid < 0))
			p->pid = 0;

	if((cfg->slippery & nextlevel) && !(cfg->slippery & currlevel)) {
		/* nextlevel is slippery, turn back to currlevel */
		swapi(&currlevel, &nextlevel);
		/* We've got to make sure pollfds will return immediately. */
		timetowait = 0;
	} else {
		currlevel = nextlevel;
	} if(currlevel == (1<<0)) {
		/* Runlevel 0 (that's 1<<0 = 1) is "slippery" in its own particular way:
		   once it's reached, we've got to kill all that's left and exit
		   the main loop. Since shouldberunning always fails against all-0-bits,
		   this is a simple way to ensure no processes are left behind. */
		nextlevel = 0;
		timetowait = 0;
	}
}

/* Should we start $p for nextlevel?

        PRIMASK:	      xxxxxxxxxx
	nextlevel:	--dc--------3---
   	p->rlvl:	---c-a----5432--
	SUBMASK:        xxxxxx

   To say yes, we need to make sure there'a a bit for
   current primary runlevel, and that if there are bits
   set for sublevels, at least one of them matches a bit
   in nextlevel.

   Note (p->rlvl & SUBMASK == 0) means "disregard sublevels",
   but (p->rlvl & SUBMASK == SUBMASK) means "run in *any* sublevel" and excludes
   no-active-sublevels case. */

static inline int shouldberunning(struct initrec* p)
{
	if(!(p->rlvl & nextlevel & PRIMASK))
		/* not in this primary level */
		return 0;
	if(p->flags & P_MANUAL)
		/* manually enabled, ignore sublevels */
		/* manual disable drops bit from rlvl */
		return 1;
	if(!(p->rlvl & SUBMASK))
		/* sublevels not defined = run in all sublevels */
		return 1;
	if(!(p->rlvl & nextlevel & SUBMASK))
		return 0;

	return 1;
}

/* check whether $$last was at least $wait seconds ago; if not, update timetowait
   and write a message */
static int waitneeded(struct initrec* p, time_t* last, time_t wait, const char* msg)
{
	time_t curtime = monotime();
	time_t endtime = *last + wait;

	if(endtime <= curtime) {
		*last = curtime;
		return 0;
	} else {
		int ttw = endtime - curtime;
		if(timetowait < 0 || timetowait > ttw)
			timetowait = ttw;
		retwarn(1, "%s[%i] waiting %i seconds before %s", p->name, p->pid, ttw, msg);
	}
}

/* both spawn() and stop() should check relevant timeouts, do their resp.
   actions if that's ok to do, or update timetowait via waitneeded call
   to ensure initpass() will be performed once the timeout expires */
static void spawn(struct initrec* p)
{
	int pid;

	if(p->pid > 0) {
		/* this is not right, spawn() should only be called
		   for entries that require starting */
		warn("%s[%i] can't spawn, it's already running", p->name, p->pid);
		return;
	}

	if(waitneeded(p, &p->lastrun, cfg->time_to_restart, "restarting"))
		return;

	pid = fork();
	if(pid < 0) {
		warn("%s[*] can't fork: %m", p->name);
		p->lastrun = monotime();
		return;
	}

	if(pid > 0) {
		p->pid = pid;
		p->lastrun = monotime();
		warn("%s[%i] spawned", p->name, p->pid);
		return;
	}

	/* ok, we're in the child process */
	close(initctlfd);
	execinitrec(p);
}

void stop(struct initrec* p)
{
	if(p->pid <= 0) {
		warn("#%s: attempted to stop process that's not running", p->name);
		return;
	}

	if(p->flags & P_ZOMBIE) {
		warn("#%s[%i] SIGKILL already sent", p->name, p->pid);
		return;
	} else if(p->flags & P_SIGKILL) {
		if(waitneeded(p, &p->lastsig, cfg->time_to_skip, "skipping"))
			return;
		warn("#%s[%i] process refuses to die after SIGKILL, skipping", p->name, p->pid);
		p->pid = 0;
		p->flags |= P_ZOMBIE;
		p->flags &= ~(P_SIGKILL | P_SIGTERM);
	} else if(p->flags & P_SIGTERM) {
		if(waitneeded(p, &p->lastsig, cfg->time_to_SIGKILL, "sending SIGKILL"))
			return;
		warn("#%s[%i] sending SIGKILL", p->name, p->pid);
		kill(p->pid, SIGKILL);
		p->flags |= P_SIGKILL;
	} else {
		warn("#%s[%i] terminating", p->name, p->pid);
		kill(p->pid, (p->flags & C_USEABRT ? SIGABRT : SIGTERM));
		p->flags |= P_SIGTERM;

		/* Attempt to wake the process up to recieve SIGTERM. */
		/* This must be done *after* sending the killing signal
		   to ensure SIGCONT does not arrive first. */
		if(p->flags & P_SIGSTOP)
			kill(p->pid, SIGCONT);

		/* make sure we'll get initpass to send SIGKILL if necessary */
		if(timetowait < 0 || timetowait > cfg->time_to_SIGKILL)
			timetowait = cfg->time_to_SIGKILL;
	}
}

static inline void swapi(int* a, int* b)
{
	int t = *b; *b = *a; *a = t;
}

/* Kernels below 2.6.something have no CLOCK_BOOTTIME defined */
#ifdef CLOCK_BOOTTIME
#define INIT_CLOCK CLOCK_BOOTTIME
#else
#define INIT_CLOCK CLOCK_MONOTONIC
#endif

/* At bootup, the system starts with all lastrun=0 and possibly also
   with the clock near 0, activating time_to_* timers even though
   no processes have been started at time 0. To avoid delays, monotonic
   clocks are shifted forward so that boot occurs at some time past 0.

   The shift could have been as small as time_to_restart, but alas,
   that is a config variable which may change, breaking monotonic
   constraint. So instead it's set to the maximum possible ttr value.

   Given ttr is just a short, and monotonic clocks always start at 0
   at bootup, the resulting number is still smaller than even wall
   clock value (as in time(2)) */
#define INIT_CLOCK_OFFSET 0xFFFF

/* monotonic equivalent of time(NULL) */
static time_t monotime(void)
{
	struct timespec tp;

	if(clock_gettime(INIT_CLOCK, &tp))
		return 0;
	
	return tp.tv_sec + INIT_CLOCK_OFFSET;
}
