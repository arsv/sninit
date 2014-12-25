#include <signal.h>
#include <unistd.h>

/* libgcc relies on this function in some cases */
/* (ARM: signal FPE on divide by zero) */

int raise(int sig)
{
	return kill(getpid(), sig);
}
