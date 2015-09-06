#include <unistd.h>
#include <sys/wait.h>
#include "init.h"
#include "scope.h"

extern int state;
extern int nextlevel;
extern struct config* cfg;
extern time_t passtime;

export void waitpids(void);

local void markdead(struct initrec* p, int status);
local void markstopped(struct initrec* p, int status);

local void checkfailacts(struct initrec* p, int failed);
local void faildisable(struct initrec* p);

/* So we were signalled SIGCHLD and got to check what's up with
   the children. And because we care about killing paused (as in SIGSTOP)
   children gracefully, we want to track running/stopped status as well.
   Linux allows this with WUNTRACED and WCONTINUED. */

void waitpids(void)
{
	pid_t pid;
	int status;
	struct initrec *p, **pp;
	const int flags = WNOHANG | WUNTRACED | WCONTINUED;

	while((pid = waitpid(-1, &status, flags)) > 0)
	{
		for(pp = cfg->inittab; (p = *pp); pp++)
			if(p->pid == pid)
				break;
		if(!p)	/* Some stray child died. Like we care. */
			continue;

		if(WIFSTOPPED(status))
			markstopped(p, status);
		else
			markdead(p, status);
	}

	state &= ~S_SIGCHLD;
}

/* For stopped childred, just note their status;
   stop() will send the stopped ones SIGCONT together with SIGTERM. */

void markstopped(struct initrec* p, int status)
{
	if(WIFCONTINUED(status))
		p->flags &= ~P_SIGSTOP;
	else
		p->flags |= P_SIGSTOP;
}

/* For dead children, there's a difference between "exited normally"
   and "exited abnormally", so we've got to decide that and, if configured,
   take action upon abnormal exit.

   The actual criteria depend on DOF/DTF flags, and may be non-zero
   return code, or too short lifetime, or both. */

void markdead(struct initrec* p, int status)
{
	int failed;

	if(p->flags & (P_SIGTERM | P_SIGKILL))
		failed = 0; /* requested exit is always correct */
	else
		failed = (!WIFEXITED(status) || WEXITSTATUS(status));

	if(failed && !(p->flags & C_HUSH))
		warn("%s[%i] abnormal exit %i", p->name, p->pid,
			WIFEXITED(status) ? WEXITSTATUS(status) : -WTERMSIG(status));

	if(p->flags & (C_DOF | C_DTF))
		checkfailacts(p, failed);

	/* mark the entry as safely dead */
	p->pid = -1;
	p->flags &= ~(P_SIGTERM | P_SIGKILL);
}

void checkfailacts(struct initrec* p, int failed)
{
	if(p->flags & C_DTF) {

		int mintime = (p->flags & C_FAST ? TIME_TO_RESTART : MINIMUM_RUNTIME);
		int toofast = (passtime - p->lastrun <= mintime);

		/* fast respawning only counts when exit status is nonzero
		   if C_DOF is set; without C_DOF, all exits are counted. */
		if(p->flags & C_DOF)
			failed = toofast && failed;
		else
			failed = toofast;

		if(failed && (p->flags & P_WAS_OK))
			p->flags &= ~P_WAS_OK;
		else if(failed)
			faildisable(p);
		else
			p->flags |= P_WAS_OK;

	} else if(failed && p->flags & C_DOF)
		faildisable(p);
}

void faildisable(struct initrec* p)
{
	warn("%s[%i] failed, disabling", p->name, p->pid);
	p->flags |= P_FAILED;
}
