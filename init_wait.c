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
local void checkfailacts(struct initrec* p, int failed);
local void faildisable(struct initrec* p);
local void failswitch(struct initrec* p);

/* we were signalled SIGCHLD, got to mark died processes as such */
void waitpids(void)
{
	pid_t pid;
	int status;
	struct initrec *p, **pp;

	while((pid = waitpid(-1, &status, WNOHANG)) > 0)
		for(pp = cfg->inittab; (p = *pp); pp++)
			if(p->pid == pid)
				markdead(p, status);

	state &= ~S_SIGCHLD;
}

void markdead(struct initrec* p, int status)
{
	int failed;

	if(WIFSTOPPED(status)) {
		/* It is quite possible SIGSTOP was not sent by init,
		   which would mean P_SIGSTOP is not in flags even though
		   the process is stopped and must be woken up in stop() */
		p->flags |= P_SIGSTOP;
		/* Other than that, we are no interested in stopped processes */
		return;
	}

	if(p->flags & (P_SIGTERM | P_SIGKILL))
		failed = 0; /* requested exit is always correct */
	else
		failed = (!WIFEXITED(status) || WEXITSTATUS(status));

	if(failed && !(p->flags & C_HUSH))
		warn("%s[%i] abnormal exit %i", p->name, p->pid,
			WIFEXITED(status) ? WEXITSTATUS(status) : -WTERMSIG(status));

	if(p->flags & (C_DOF | C_DTF | C_ROFa | C_ROFb))
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

	/* Use non-zero exit status *unless* DTF is given, in which case
	   follow DTF logic (i.e. runlevel switch only if failed too fast) */
	if(failed && (p->flags & (C_ROFa | C_ROFb)))
		failswitch(p);
}

void faildisable(struct initrec* p)
{
	warn("%s[%i] failed, disabling", p->name, p->pid);
	p->flags |= P_FAILED;
}

void failswitch(struct initrec* p)
{
	int lvl = 0;
	switch(p->flags & (C_ROFa | C_ROFb)) {
		case C_ROFa: lvl = FALLBACK1; break;
		case C_ROFb: lvl = FALLBACK2; break;
		case C_ROFa | C_ROFb: lvl = FALLBACK3; break;
		default: return;
	}

	int next = (nextlevel & SUBMASK) | (1 << lvl);
	if(next == nextlevel)
		return; /* already going there */

	warn("%s[%i] failure prompts runlevel switch to %i", p->name, p->pid, lvl);
	nextlevel = next;
}
