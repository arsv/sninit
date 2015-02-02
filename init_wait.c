#include <unistd.h>
#include <sys/wait.h>
#include "init.h"

extern int state;
extern int nextlevel;
extern struct config* cfg;

#define hush(p) (p->flags & C_HUSH)

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

			/* do on-fail actions */
			if(!WIFEXITED(status) || WEXITSTATUS(status)) {
				if(p->flags & C_DOF)
					faildisable(p);
				if(p->flags & (C_ROFa | C_ROFb))
					failswitch(p);
			}

			/* mark the entry as safely dead */
			p->pid = -1;
			p->flags &= ~(P_SIGTERM | P_SIGKILL | P_ZOMBIE);
		}
	}

	state &= ~S_SIGCHLD;
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
