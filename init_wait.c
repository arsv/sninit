#include <unistd.h>
#include <sys/wait.h>
#include "init.h"

extern int state;
extern int nextlevel;
extern struct config* cfg;
extern time_t passtime;

#define hush(p) (p->flags & C_HUSH)

static void checkfailure(struct initrec* p, int status);
static void faildisable(struct initrec* p);
static void failswitch(struct initrec* p);

/* we were signalled SIGCHLD, got to mark died processes as such */
void waitpids(void)
{
	pid_t pid;
	int status;
	struct initrec *p, **pp;
	int v;

	while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		for(pp = cfg->inittab; (p = *pp); pp++) {
			if(p->pid != pid)
				continue;

			/* report failure if necessary */
			if(WIFSTOPPED(status)) {
				/* XXX: maybe set P_SIGSTOP here?.. */
				continue;
			} else if(WIFEXITED(status)) {
				if((v = WEXITSTATUS(status)) && !hush(p))
					warn("%s[%i] abnormal exit (%i)", p->name, p->pid, v);
			} else if(WIFSIGNALED(status)) {
				if((v = WTERMSIG(status)) && !(p->flags & P_SIGTERM) && !hush(p))
					warn("%s[%i] killed by signal %i", p->name, p->pid, WTERMSIG(status));
				/* well, it could have been some other signal, but hey,
				   if init was trying to kill it anyway, who cares why it died */
			}

			if(p->flags & (C_DOF | C_DTF | C_ROFa | C_ROFb))
				checkfailure(p, status);

			/* mark the entry as safely dead */
			p->pid = -1;
			p->flags &= ~(P_SIGTERM | P_SIGKILL);
		}
	}

	state &= ~S_SIGCHLD;
}

static void checkfailure(struct initrec* p, int status)
{
	int failed = (!WIFEXITED(status) || WEXITSTATUS(status));

	if(p->flags & (C_ROFa | C_ROFb))
		if(failed)
			failswitch(p);

	if(p->flags & C_DTF) {

		int toofast = (passtime - p->lastrun <= cfg->time_to_restart);

		if(p->flags & C_DOF)
			/* fast respawning only counts when exit status is nonzero
			   if C_DOF is set; without C_DOF, all exits are counted. */
			toofast = toofast && failed;

		if(toofast && (p->flags & P_WAS_OK))
			p->flags &= ~P_WAS_OK;
		else if(toofast)
			faildisable(p);
		else
			p->flags |= P_WAS_OK;

	} else if(p->flags & C_DOF) {
		if(failed)
			faildisable(p);
	}
}

static void faildisable(struct initrec* p)
{
	warn("%s[%i] failed, disabling", p->name, p->pid);
	p->flags |= P_FAILED;
}

static void failswitch(struct initrec* p)
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
