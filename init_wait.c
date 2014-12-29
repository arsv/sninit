#include <unistd.h>
#include <sys/wait.h>
#include "init.h"

extern int state;
extern struct config* cfg;

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

			if(WIFSTOPPED(status)) {
				/* XXX: maybe set P_SIGSTOP here?.. */
				continue;
			} else if(WIFEXITED(status)) {
				if((v = WEXITSTATUS(status)))
					warn("%s[%i] abnormal exit (%i)", p->name, p->pid, v);
			} else if(WIFSIGNALED(status)) {
				if((v = WTERMSIG(status)) && !(p->flags & P_SIGTERM))
					warn("%s[%i] killed by signal %i", p->name, p->pid, WTERMSIG(status));
				/* well, it could have been some other signal, but hey,
				   if init was trying to kill it anyway, who cares why it died */
			}

			p->pid = -1;
			p->flags &= ~(P_SIGTERM | P_SIGKILL | P_ZOMBIE);
		}
	}

	state &= ~S_SIGCHLD;
}
