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
	struct initrec* p;

	while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		for(p = cfg->inittab; p; p = p->next) {
			if(p->pid != pid)
				continue;
			if(!(p->flags & C_WAIT))
				warn("%s[%i] died (%i)", p->name, p->pid, status);
			else if(status)
				warn("%s[%i] abnormal exit (%i)", p->name, p->pid, status);
			p->pid = -1;
			p->flags &= ~(P_SIGTERM | P_SIGKILL | P_ZOMBIE);
		}
	}

	state &= ~S_SIGCHLD;
}
