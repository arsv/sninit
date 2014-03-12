#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "init.h"

/* bits for waitfor */
#define DYING		(1<<0)
#define RUNNING		(1<<1)
#define DISABLED	(1<<2)

extern int state;
extern int currlevel;
extern int nextlevel;
extern int timetowait;
extern int initctlfd;
extern struct config* cfg;

extern void sexec(struct initrec* p);

static void spawn(struct initrec* p);
global void stop(struct initrec* p);

static inline void swapi(int* a, int* b);
static inline int shouldberunning(struct initrec* p);

/* Initpass: go through inittab, top-to-bottom, (re)starting entries
   that need to be (re)started and killing entries that should be killed.

   Each pass is always started at the top of inittab, even if previous one
   ended somewhere in the middle at a w-type entry. This approach adds
   some overhead for regular runlevel switching, but simplifies services
   restarts and seamlessly handles changes in nextlevel midway.

   In case some w-type entry is reached, initpass spawns it and returns.
   SIGCHLD will arrive on entry completition, triggering another initpass.
   Blocking wait is never used, sinit waits for w-type entries in ppoll().
 
   Because no explicit list pointer is kept during runlevel switching,
   things get a bit tricky with w-/o-type entries which are traversed
   several times but should only be run once. Note to have pid reset to 0,
   and thus allow re-run, at least one pass should be performed
   with !shouldberunning(p) */

void initpass(void)
{
	int waitfor = 0;
	struct initrec* p;

	state |= S_WAITING;
	for(p = cfg->inittab; p; p = p->next) {
		if(!shouldberunning(p)) {
			if(p->pid > 0) {
				stop(p);
				waitfor |= DYING;
			} else {
				/* for w-type processes, to run them again
				   in a switch to an appropriate runlevel will occur later */
				p->pid = 0;
			}
		} else if(waitfor & DYING) {
			/* something from currlevel is not dead yet, do not hurry with nextlevel */
		} else if(p->pid > 0) {
			/* process is running and it's ok */
			if(p->flags & C_WAIT)
				return;
			if(p->flags & C_ONCE)
				waitfor |= RUNNING;
		} else if(p->pid < 0 && (p->flags & (C_ONCE | C_WAIT))) {
			/* has been run already */
		} else if(waitfor && (p->flags & C_WAIT)) {
			/* wait for o-entries before starting a w-entry */
			return;
		} else {
			/* ok, now we're absolutely sure $p should be spawned */
			spawn(p);

			if(p->flags & C_ONCE)
				waitfor |= RUNNING;	/* we're not in nextlevel yet */
			if(p->flags & C_WAIT)
				return;			/* off to wait for this process */
		}
	}

	if(!waitfor)
		state &= ~S_WAITING;
 	if(waitfor || currlevel == nextlevel)
		return;

	/* nothing more to run, we've done switching runlvls */

	if((cfg->slippery & nextlevel) && !(cfg->slippery & currlevel)) {
		/* nextlevel is slippery, turn back to currlevel */
		swapi(&currlevel, &nextlevel);
		/* We've got to make sure pollfds will return immediately. */
		/* Something like SIGUSR1 would have worked as well, but SIGCHLD
		   is enough for the purporse and there's already a handler */
		kill(getpid(), SIGCHLD);
	} else {
		currlevel = nextlevel;
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
	if(p->flags & P_DISABLED)
		return 0;

	if(!(p->rlvl & nextlevel & PRIMASK))
		/* not in this primary level */
		return 0;
	if(!(p->rlvl & SUBMASK))
		/* sublevels not defined = run in all sublevels */
		return 1;
	if(!(p->rlvl & nextlevel & SUBMASK))
		return 0;

	return 1;
}

/* check whether *last was at least $wait seconds ago; if not, update timetowait
   and write a message */
static int waitneeded(struct initrec* p, time_t* last, time_t wait, const char* msg)
{
	time_t curtime = time(NULL);
	int ttw = *last + wait - curtime;

	if(ttw <= 0) {
		*last = curtime;
		return 0;
	} else {
		if(timetowait < 0 || timetowait > ttw) timetowait = ttw;
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
		warn("%s: can't fork: %m", p->name);
		p->lastrun = time(NULL);
		return;
	}

	if(pid > 0) {
		p->pid = pid;
		p->lastrun = time(NULL);
		return;
	}

	/* ok, we're in the child process */
	close(initctlfd);
	sexec(p);
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
		warn("#%s[%i]: process refuses to die after SIGKILL, skipping", p->name, p->pid);
		p->pid = 0;
		p->flags |= P_ZOMBIE;
		p->flags &= ~(P_SIGKILL | P_SIGTERM);
	} else if(p->flags & P_SIGTERM) {
		if(waitneeded(p, &p->lastsig, cfg->time_to_SIGKILL, "sending SIGKILL"))
			return;
		warn("#%s[%i]: sending SIGKILL", p->name, p->pid);
		kill(p->pid, SIGKILL);
		p->flags |= P_SIGKILL;
	} else {
		warn("#%s[%i]: terminating", p->name, p->pid);
		kill(p->pid, (p->flags & C_USEABRT ? SIGABRT : SIGTERM));
		p->flags |= P_SIGTERM;
		/* make sure we'll get initpass to send SIGKILL if necessary */
		if(timetowait < 0 || timetowait > cfg->time_to_SIGKILL)
			timetowait = cfg->time_to_SIGKILL;
	}
}

static inline void swapi(int* a, int* b)
{
	int t = *b; *b = *a; *a = t;
}
