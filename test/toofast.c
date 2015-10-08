#include <signal.h>
#include "../init.h"
#include "../config.h"
#include "_test.h"

/* Test handling of fast-respawning entries in waitpids().
   This could have been a part of t_waitpids, but the tests
   are quite lengthy and unlike the rest of waipids do need
   nontrivial passtime. */

#define SLOWENTRY 0
#define FASTENTRY C_FAST

int state;
int nextlevel;
time_t passtime;

struct initrec I;
struct initrec* testinittab[] = { NULL, &I, NULL };
struct config testconfig = {
	.inittab = testinittab + 1,
	.initnum = sizeof(testinittab)/sizeof(void*) - 2,
};
struct config* cfg = &testconfig;

/* return status: 0xAABB, AA = exit code, BB = signal */
/* BB = 0x00 means exited, not killed */
/* BB = 0x7F means stopped */
#define EXIT(n) ((n & 0xFF) << 8)
#define KILL(n) (n & 0xFF)

int wait_pid;
int wait_ret;

int waitpid(int p, int* status, int flags)
{
	int ret = wait_pid;
	*status = wait_ret;
	wait_pid = -1;
	return ret;
}

extern void waitpids(void);

/* start and stop times */
#define START 1000
#define FAST (START + MINIMUM_RUNTIME/2)
#define SLOW (START + 2*MINIMUM_RUNTIME)

void run(int flags, time_t howfast, int status)
{
	static int nprocs = 37;

	I.pid = nprocs++;
	wait_pid = I.pid;
	wait_ret = status;
	I.flags = flags;
	I.lastrun = START;
	passtime = howfast;

	waitpids();
}

int main(void)
{
	/* slow normal exit */
	run(SLOWENTRY, SLOW, EXIT(0x00));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY | P_WAS_OK));

	/* slow abnormal exit */
	run(SLOWENTRY, SLOW, EXIT(0xFF));
	A(I.pid == -1);
	A(I.flags == SLOWENTRY);

	/* 0-exits are always ok, even if fast */
	run(SLOWENTRY, FAST, EXIT(0x00));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY | P_WAS_OK));

	/* non-zero exit w/o P_WAS_OK, disable it */
	run(SLOWENTRY, FAST, EXIT(0x01));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY | P_FAILED));

	/* same, killed by a bad signal */
	run(SLOWENTRY, FAST, KILL(SIGSEGV));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY | P_FAILED));

	/* same, killed by a good signal */
	run(SLOWENTRY, FAST, KILL(SIGTERM));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY | P_WAS_OK));	/* XXX: maybe skip P_WAS_OK here? */

	/* non-zero exit w/o P_WAS_OK, if done slowly, is ok */
	run(SLOWENTRY, SLOW, EXIT(0x01));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY));

	/* non-zero exit w/ P_WAS_OK only clears that flag */
	run(SLOWENTRY | P_WAS_OK, SLOW, EXIT(0x12));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY));

	run(SLOWENTRY | P_WAS_OK, FAST, KILL(SIGSEGV));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY));

	run(SLOWENTRY | P_WAS_OK, FAST, EXIT(0x13));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY));

	run(SLOWENTRY | P_WAS_OK, FAST, KILL(SIGSEGV));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY));

	/* killing entry with a good signal does not clear P_WAS_OK */
	run(SLOWENTRY | P_WAS_OK, FAST, KILL(SIGTERM));
	A(I.pid == -1);
	A(I.flags == (SLOWENTRY | P_WAS_OK));	/* XXX: maybe clear it here? */

	return 0;
}
