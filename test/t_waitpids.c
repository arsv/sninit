#include <stdlib.h>
#include <string.h>
#include "../init.h"
#include "test.h"

int state;
int nextlevel;

/* Startup: I1 * I2 * I3 I4 I5 * rlswitch */
struct initrec I0 = { .pid = 11, .flags = 0 };
struct initrec I1 = { .pid = 22, .flags = C_USEABRT | P_SIGTERM };
struct initrec I2 = { .pid = 33, .flags = P_SIGTERM | P_SIGKILL };
struct initrec I3 = { .pid = 44, .flags = 0 };
struct initrec I4 = { .pid = 55, .flags = C_ROFa };
struct initrec I5 = { .pid = 66, .flags = C_DOF };
struct initrec I6 = { .pid = 77, .flags = 0 };

struct initrec* testinittab[] = { NULL, &I0, &I1, &I2, &I3, &I4, &I5, &I6, NULL };
struct config testconfig = { .inittab = testinittab + 1, .initnum = sizeof(testinittab)/sizeof(void*) - 2 };

struct config* cfg = &testconfig;

/* return status: 0xAABB, AA = exit code, BB = signal */
/* BB = 0x00 means exited, not killed */
/* BB = 0x7F means stopped */
struct waitret {
	int pid;
	int status;
} waitret[] = {
	{ 11, 0x0000 }, /* I0 died on its own */
	{ 33, 0x0013 }, /* I2 got killed */
	{ 22, 0x0014 }, /* I1 got killed as well */
	/* 44 is not in the list */
	{ 66, 0x1100 }, /* I5 exited abnormally */
	{ 55, 0x0013 }, /* I4 got killed */
	{ 77, 0x1300 }, /* I6 exited abnormally */
	{  0, 0 }
};

struct waitret* waitptr = waitret;
int waitcnt = 0;

extern void waitpids(void);

int waitpid(int pid, int* status, int flags)
{
	int ret = waitptr->pid;

	if(!waitcnt)
		return -1;
	else
		waitcnt--;

	if(!ret)
		return -1;

	*status = waitptr->status;
	waitptr++;

	return ret;
}

#define R1 (1 << 1)
#define R6 (1 << 6)
#define Ra (1 << 10)
#define Rb (1 << 11)

int main(void)
{
	/* Simple common case, I0 and I2 die at the same time */
	/* Make sure they get marked, and nothing else is changed */
	state = S_SIGCHLD;
	waitcnt = 2;
	waitpids();
	A(I0.pid == -1);
	A(I1.pid == 22);
	A(I2.pid == -1);
	A(I2.flags == 0);
	A(I3.pid == 44);
	A(I4.pid == 55);
	A(I5.pid == 66);
	A(I6.pid == 77);
	A(state == 0);

	/* Are signal flags removed properly? */
	waitcnt = 1;
	waitpids();
	A(I1.pid == -1);
	A(I1.flags == (C_USEABRT));

	/* C_DOF test */
	waitcnt = 1;
	waitpids();
	A(I5.pid == -1);
	A(I5.flags == (C_DOF | P_FAILED));

	/* C_ROF test */
	nextlevel = R6 | Ra | Rb;
	waitcnt = 1;
	waitpids();
	A(I4.pid == -1);
	A(I4.flags == C_ROFa);
	A(nextlevel == (R1 | Ra | Rb));

	/* Just a simple abnormal exit */
	waitcnt = 1;
	waitpids();
	A(I6.pid == -1);
	A(I6.flags == 0);

	/* Dry run */
	state = S_SIGCHLD;
	waitcnt = 0;
	waitpids();
	A(state == 0);

	return 0;
}
