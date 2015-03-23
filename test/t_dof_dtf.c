#include <stdlib.h>
#include <string.h>
#include "../init.h"
#include "../config.h"
#include "test.h"

/* Test C_DOF and C_DTF handling in waitpids().
   This could have been a part of t_waitpids, but the tests
   are quite lengthy and unlike the rest of waipids do need
   nontrivial passtime. */

int state;
int nextlevel;
time_t passtime;

struct initrec I1 = { .pid = 11, .lastrun = 0, .flags = 0 };
struct initrec I2 = { .pid = 22, .lastrun = 0, .flags = C_DOF };
struct initrec I3 = { .pid = 33, .lastrun = 0, .flags = C_DTF };
struct initrec I4 = { .pid = 44, .lastrun = 0, .flags = C_DOF | C_DTF };

struct initrec* testinittab[] = { NULL, &I1, &I2, &I3, &I4, NULL };
struct config testconfig = {
	.inittab = testinittab + 1,
	.initnum = sizeof(testinittab)/sizeof(void*) - 2,
};

struct config* cfg = &testconfig;

extern void waitpids(void);

int wait_pid = 0;
int wait_ret = 0;
/* return status: 0xAABB, AA = exit code, BB = signal */
/* BB = 0x00 means exited, not killed */
/* BB = 0x7F means stopped */

int waitpid(int p, int* status, int flags)
{
	int pid = wait_pid;

	if(!pid) {
		return -1;
	} else {
		*status = wait_ret;
		wait_pid = 0;
		return pid;
	}
}

/* start and stop times */
#define START 1000
#define FAST (START + MINIMUM_RUNTIME/2)
#define SLOW (START + 2*MINIMUM_RUNTIME)

void dieprocdie(struct initrec* p, int status, time_t t1)
{
	wait_pid = p->pid;
	wait_ret = status;
	p->lastrun = START;
	passtime = t1;

	waitpids();
}

void resetproc(struct initrec* p)
{
	struct initrec** q;
	
	for(q = testinittab + 1; *q; q++)
		if(*q == p) {
			p->pid = q - testinittab;
			p->pid += 10*p->pid;
			break;
		}
	if(!*q)
		p->pid = 1;

	p->flags &= ~(P_FAILED | P_WAS_OK);
}

void test_plain(void)
{
	dieprocdie(&I1, 0xFF00, SLOW);
	A(I1.pid == -1);
	A(I1.flags == 0);
	resetproc(&I1);
}

void test_dof(void)
{
	/* simple DOF, normal exit, no action */
	A(I2.flags == C_DOF);
	dieprocdie(&I2, 0x0000, SLOW);
	A(I2.pid == -1);
	A(I2.flags == C_DOF);
	resetproc(&I2);

	/* simple DOF, failure exit, disable */
	A(I2.flags == C_DOF);
	dieprocdie(&I2, 0xFF00, SLOW);
	A(I2.pid == -1);
	A(I2.flags == (C_DOF | P_FAILED));
	resetproc(&I2);
}

void test_dtf(void)
{
	/* simple DTF, normal slow exit, set WAS_OK */
	A(I3.flags == C_DTF);
	dieprocdie(&I3, 0x0000, SLOW);
	A(I3.pid == -1);
	A(I3.flags == (C_DTF | P_WAS_OK));
	resetproc(&I3);

	/* simple DTF, abnormal slow exit, set WAS_OK */
	A(I3.flags == C_DTF);
	dieprocdie(&I3, 0xFF00, SLOW);
	A(I3.pid == -1);
	A(I3.flags == (C_DTF | P_WAS_OK));
	resetproc(&I3);

	/* simple DTF, normal fast exit without WAS_OK, disable */
	A(I3.flags == C_DTF);
	dieprocdie(&I3, 0x0000, FAST);
	A(I3.pid == -1);
	A(I3.flags == (C_DTF | P_FAILED));
	resetproc(&I3);

	/* simple DTF, normal fast exit with WAS_OK, remove WAS_OK */
	I3.flags |= P_WAS_OK;
	A(I3.flags == (C_DTF | P_WAS_OK));
	dieprocdie(&I3, 0x0000, FAST);
	A(I3.pid == -1);
	A(I3.flags == C_DTF);
	resetproc(&I3);
}

void test_dof_dtf(void)
{
	/* DOF | DTF, normal slow exit, set WAS_OK */
	A(I4.flags == (C_DOF | C_DTF));
	dieprocdie(&I4, 0x0000, SLOW);
	A(I4.pid == -1);
	A(I4.flags == (C_DOF | C_DTF | P_WAS_OK));
	resetproc(&I4);

	/* DOF | DTF, abnormal slow exit, set WAS_OK */
	A(I4.flags == (C_DOF | C_DTF));
	dieprocdie(&I4, 0xFF00, SLOW);
	A(I4.pid == -1);
	A(I4.flags == (C_DOF | C_DTF | P_WAS_OK));
	resetproc(&I4);

	/* DOF | DTF, normal fast exit without WAS_OK, set WAS_OK */
	A(I4.flags == (C_DOF | C_DTF));
	dieprocdie(&I4, 0x0000, FAST);
	A(I4.pid == -1);
	A(I4.flags == (C_DOF | C_DTF | P_WAS_OK));
	resetproc(&I4);

	/* DOF | DTF, normal fast exit with WAS_OK, do nothing */
	I4.flags |= P_WAS_OK;
	A(I4.flags == (C_DOF | C_DTF | P_WAS_OK));
	dieprocdie(&I4, 0x0000, FAST);
	A(I4.pid == -1);
	A(I4.flags == (C_DOF | C_DTF | P_WAS_OK));
	resetproc(&I4);

	/* DOF | DTF, abnormal fast exit without WAS_OK, disable */
	A(I4.flags == (C_DOF | C_DTF));
	dieprocdie(&I4, 0xFF00, FAST);
	A(I4.pid == -1);
	A(I4.flags == (C_DOF | C_DTF | P_FAILED));
	resetproc(&I4);

	/* DOF | DTF, abnormal fast exit with WAS_OK, remove WAS_OK */
	I4.flags |= P_WAS_OK;
	A(I4.flags == (C_DOF | C_DTF | P_WAS_OK));
	dieprocdie(&I4, 0xFF00, FAST);
	A(I4.pid == -1);
	A(I4.flags == (C_DOF | C_DTF));
	resetproc(&I4);
}

int main(void)
{
	test_plain();
	test_dof();
	test_dtf();
	test_dof_dtf();

	return 0;
}
