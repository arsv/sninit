#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "init.h"
#include "scope.h"
#include "sys.h"

/* Termination by certain signals is not considered a failure,
   because it is nearly always the user who sends the signal. */

static int badsignal(int sig)
{
	switch(sig) {
		case SIGINT: return 0;
		case SIGTERM: return 0;
		default: return 1;
	}
}

/* An entry that should not exit yet somehow keeps respawning is
   probably misconfigured (missing executable, config file errors
   and such). We would like to detect and disable such entries.

   At the same time, we should not prevent respawns on occassional
   errors, which is what respawning entries are for.

   There is no strict way to tell the two cases apart. A simple rule
   of thumb is used instead: a process that has been running for at
   least MINIMUM_RUNTIME is assumed to have passed initial configuration
   stage and is eligible for respawning.

   At boot, all entries are in a neutral state: neither failed nor
   ever run successfully. This gives them a single shot at starting,
   i.e. the first attempt must be at least MINIMUM_RUNTIME long or they
   will get disabled immediately. */

static void checktoofast(struct initrec* p, int status)
{
	int failed;

	if(WIFSIGNALED(status))
		failed = badsignal(WTERMSIG(status));
	else
		failed = WEXITSTATUS(status);

	int toofast = (passtime - p->lastrun <= MINIMUM_RUNTIME);

	if(!failed) {
		p->flags |= P_WAS_OK;
	} else if(p->flags & P_WAS_OK) {
		p->flags &= ~P_WAS_OK;
	} else if(toofast) {
		warn("%s[%i] respawning too fast, disabling", p->name, p->pid);
		p->flags |= P_FAILED;
	}
}

/* For entries killed by a singal, let's report readable singal name.
   This is probably one of the least important things in init, given
   how unlikely signal kills are normally.
   But we do report errno values, and it's about the same level of verbosity.

   We are only interested in signals capable of terminating
   the process. That's Term or Core in signal(7). */

static struct signame {
	short sig;
	char name[6];
} signames[] = {
#define SIG(s) { SIG##s, #s }
	SIG(HUP),
	SIG(INT),
	SIG(QUIT),
	SIG(ILL),
	SIG(ABRT),
	SIG(FPE),
	SIG(PIPE),
	SIG(ALRM),
	SIG(TERM),
	SIG(USR1),
	SIG(USR2),
	SIG(BUS),
	SIG(XCPU),
	SIG(XFSZ)
};

static const char* signalname(int sig)
{
	struct signame* p;
	struct signame* e = signames + sizeof(signames)/sizeof(*signames);

	for(p = signames; p < e; p++)
		if((int)p->sig == sig)
			return p->name;

	return NULL;
}

static void reportdeath(struct initrec* p, int status)
{
	const char* signame;

	if(WIFEXITED(status))
		warn("%s[%i] abnormal exit %i", p->name, p->pid, WEXITSTATUS(status));
	else if(!WIFSIGNALED(status))
		return;
	else if((signame = signalname(WTERMSIG(status))))
		warn("%s[%i] killed by SIG%s", p->name, p->pid, signame);
	else
		warn("%s[%i] killed by signal %i", p->name, p->pid, WTERMSIG(status));
}

/* Abnormal exits should be reported, and too fast respawns taken
   actions upon. It is not a simple 0-return check however: disabling
   entries likely killed by the user is not a good idea, and entries
   killed by init should not be reported. */

static void markdead(struct initrec* p, int status)
{
	if(WIFEXITED(status) && !WEXITSTATUS(status))
		;    /* zero exits are ok */
	else if(p->flags & (P_SIGTERM | P_SIGKILL))
		;    /* expected exit, init killed it */
	else if(p->flags & C_HUSH)
		;    /* should not be reported */
	else reportdeath(p, status);

	if(p->flags & C_ONCE)
		;    /* no point in timing run-once entries */
	else if(p->flags & C_FAST)
		;    /* this entry is allowed to respawn fast */
	else if(p->flags & (P_SIGTERM | P_SIGKILL))
		;    /* expected exits should not be checked */
	else checktoofast(p, status);

	p->pid = -1;
	p->flags &= ~(P_SIGTERM | P_SIGKILL | P_SIGSTOP);
}

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
			p->flags |= P_SIGSTOP;
		else if(WIFCONTINUED(status))
			p->flags &= ~P_SIGSTOP;
		else
			markdead(p, status);
	}
}
