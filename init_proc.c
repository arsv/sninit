#include <sigset.h>
#include <time.h>
#include <bits/time.h>
#include <sys/fork.h>
#include <sys/kill.h>
#include <sys/execve.h>
#include <sys/setsid.h>
#include <sys/getpid.h>
#include "init.h"
#include "config.h"
#include "scope.h"

extern struct config* cfg;
extern int initctlfd;
extern int timetowait;
extern time_t passtime;

export void spawn(struct initrec* p);
export void stop(struct initrec* p);

local int waitneeded(time_t* last, time_t wait);

/* Both spawn() and stop() should check relevant timeouts, do their resp.
   actions if that's ok to do, or update timetowait via waitneeded call
   to ensure initpass() will be performed once the timeout expires. */

/* The code below is valid with either fork or vfork.
   Non-MMU targets must use vfork, and some MMU targets (ARM?) have troubles
   with vfork implementation, so it's fork for MMU and vfork for NOMMU. */

extern void _exit(int);

void spawn(struct initrec* p)
{
	if(p->pid > 0)
		/* this is not right, spawn() should only be called
		   for entries that require starting */
		retwarn_("%s[%i] can't spawn, already running", p->name, p->pid);

	if(waitneeded(&p->lastrun, TIME_TO_RESTART))
		return;

	pid_t pid = sysfork();

	if(pid < 0) {
		retwarn_("%s[*] can't fork: %m", p->name);
	} else if(pid > 0) {
		p->pid = pid;
		p->lastsig = 0;
		p->flags &= ~(P_SIGKILL | P_SIGTERM);
		return;
	} else {
		/* ok, we're in the child process */

		syssetsid();	/* become session *and* pgroup leader */
		/* pgroup is needed to kill(-pid), and session is important
		   for spawned shells and gettys (busybox ones at least) */

		sysexecve(p->argv[0], p->argv, cfg->env);
		warn("%s[%i] exec(%s) failed: %m", p->name, sysgetpid(), p->argv[0]);
		_exit(-1);
	}
}

void stop(struct initrec* p)
{
	if(p->pid == 0 && (p->flags & P_SIGKILL))
		/* Zombie, no need to report it */
		return;

	if(p->pid <= 0)
		/* This can only happen on telinit stop, so let the user know */
		retwarn_("%s: attempted to stop process that's not running", p->name);

	if(p->flags & P_SIGKILL) {
		/* The process has been sent SIGKILL, still refuses
		   to kick the bucket. Just forget about it then,
		   reset p->pid and let the next initpass restart the entry. */
		if(waitneeded(&p->lastsig, TIME_TO_SKIP))
			return;
		warn("%s[%i] process refuses to die after SIGKILL, skipping", p->name, p->pid);
		p->pid = 0;
	} else if(p->flags & P_SIGTERM) {
		/* The process has been signalled, but has not died yet */
		if(waitneeded(&p->lastsig, TIME_TO_SIGKILL))
			return;
		warn("%s[%i] process refuses to die, sending SIGKILL", p->name, p->pid);
		syskill(-p->pid, SIGKILL);
		p->flags |= P_SIGKILL;
	} else {
		/* Regular stop() invocation, gently ask the process to leave
		   the kernel process table */

		p->lastsig = passtime;

		int sig = p->flags & C_KILL ? SIGKILL : SIGTERM;
		int flg = p->flags & C_KILL ? P_SIGKILL : P_SIGTERM;
		syskill(-p->pid, sig);
		p->flags |= flg;

		/* Attempt to wake the process up to recieve SIGTERM. */
		/* This must be done *after* sending the killing signal
		   to ensure SIGCONT does not arrive first. */
		if(p->flags & P_SIGSTOP)
			syskill(-p->pid, SIGCONT);

		/* make sure we'll get initpass to send SIGKILL if necessary */
		if(timetowait < 0 || timetowait > TIME_TO_SIGKILL)
			timetowait = TIME_TO_SIGKILL;
	}
}

/* start() and stop() are called on each initpass for entries that should
   be started/stopped, and initpass may be triggered sooner than expected
   for unrelated reasons. So the idea is to take a look at passtime, and
   only act if the time is up, otherwise just set ppoll timer so that
   another initpass would be triggered when necessary. */

int waitneeded(time_t* last, time_t wait)
{
	time_t curtime = passtime; /* start of current initpass, see main() */
	time_t endtime = *last + wait;

	if(endtime <= curtime) {
		*last = passtime;
		return 0;
	} else {
		int ttw = endtime - curtime;
		if(timetowait < 0 || timetowait > ttw)
			timetowait = ttw;
		return 1;
	}
}
