#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "init.h"

extern struct config* cfg;
extern int initctlfd;
extern int timetowait;
extern time_t passtime;

static int waitneeded(struct initrec* p, time_t* last, time_t wait, const char* msg);

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
		warn("%s[*] can't fork: %m", p->name);
		p->lastrun = passtime;
		return;
	} else if(pid > 0) {
		p->pid = pid;
		p->lastrun = passtime;
		p->lastsig = 0;
		return;
	} else {
		/* ok, we're in the child process */
		setpgid(0, 0);
		execve(p->argv[0], p->argv, cfg->env);
		warn("%s[%i] exec(%s) failed: %m", p->name, getpid(), p->argv[0]);
		_exit(-1);
	}
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
		if(!(p->flags & C_HUSH))
			warn("#%s[%i] process refuses to die after SIGKILL, skipping", p->name, p->pid);
		p->pid = 0;
		p->flags |= P_ZOMBIE;
		p->flags &= ~(P_SIGKILL | P_SIGTERM);
	} else if(p->flags & P_SIGTERM) {
		if(waitneeded(p, &p->lastsig, cfg->time_to_SIGKILL, "sending SIGKILL"))
			return;
		if(!(p->flags & C_HUSH))
			warn("#%s[%i] sending SIGKILL", p->name, p->pid);
		kill(-p->pid, SIGKILL);
		p->flags |= P_SIGKILL;
	} else {
		if(!(p->flags & C_HUSH))
			warn("#%s[%i] terminating", p->name, p->pid);
		kill(-p->pid, (p->flags & C_USEABRT ? SIGABRT : SIGTERM));
		p->flags |= P_SIGTERM;

		/* Attempt to wake the process up to recieve SIGTERM. */
		/* This must be done *after* sending the killing signal
		   to ensure SIGCONT does not arrive first. */
		if(p->flags & P_SIGSTOP)
			kill(-p->pid, SIGCONT);

		/* make sure we'll get initpass to send SIGKILL if necessary */
		if(timetowait < 0 || timetowait > cfg->time_to_SIGKILL)
			timetowait = cfg->time_to_SIGKILL;
	}
}

/* Check whether $$last was at least $wait seconds ago; if not, update timetowait. */
static int waitneeded(struct initrec* p, time_t* last, time_t wait, const char* msg)
{
	time_t curtime = passtime;	/* time at the start of current initpass, see main() */
	time_t endtime = *last + wait;

	if(endtime <= curtime) {
		*last = curtime;
		return 0;
	} else {
		int ttw = endtime - curtime;
		if(timetowait < 0 || timetowait > ttw)
			timetowait = ttw;
		return 1;
	}
}
