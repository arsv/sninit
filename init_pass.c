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

void spawn(struct initrec* p);
void stop(struct initrec* p);

static inline void swapi(int* a, int* b);
static inline int shouldberunning(struct initrec* p);

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
			continue;
		} else if(waitfor & DYING)
			/* something from currlevel is not dead yet, do not hurry with nextlevel */
			continue;

		if(p->pid > 0) {
			/* process is running and it's ok */
			if(p->flags & C_WAIT)
				return;
			if(p->flags & C_ONCE)
				waitfor |= RUNNING;
			continue;
		}

		/* here we only have processes that are not running and do belong in this rl */

		if((p->flags & (C_ONCE | C_WAIT)) && (p->pid < 0))
			/* has been run already */
			continue;

		/* by this point only processes that need to be spawned left,
		   and we're definitely not waiting for any w-type entries */

		if(p->flags & C_WAIT && waitfor)
			/* wait for o-entries before starting a w-entry */
			return;

		spawn(p);

		if(p->flags & C_WAIT)
			return;			/* off to wait for this process */
		if(p->flags & C_ONCE)
			waitfor |= RUNNING;	/* we're not in nextlevel yet */
	}

	if(waitfor || currlevel == nextlevel)
		return;
	if(!waitfor)
		state &= ~S_WAITING;

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
	if(p->flags & C_DISABLED)
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
int waitneeded(struct initrec* p, time_t* last, time_t wait, const char* msg)
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
void spawn(struct initrec* p)
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
