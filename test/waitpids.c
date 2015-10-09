#include <stdarg.h>
#include <signal.h>
#include "../init.h"
#include "_test.h"

int state;
int nextlevel;
time_t passtime;

/* Startup: I1 * I2 * I3 I4 I5 * rlswitch */
/* C_FAST to prevent DTF checks from messing up the results;
   For DTF logic tests see toofast.c */
struct initrec I1 = { .name = "", .pid = 0 };
struct initrec I2 = { .name = "", .pid = 0 };
struct initrec I3 = { .name = "", .pid = 0 };
struct initrec I4 = { .name = "", .pid = 0 };
struct initrec I5 = { .name = "", .pid = 0 };
struct initrec I6 = { .name = "", .pid = 0 };

struct initrec* testinittab[] = { NULL, &I1, &I2, &I3, &I4, &I5, &I6, NULL };
struct config testconfig = {
	.inittab = testinittab + 1,
	.initnum = sizeof(testinittab)/sizeof(void*) - 2
};

struct config* cfg = &testconfig;

/* return status: 0xAABB, AA = exit code, BB = signal */
/* BB = 0x00 means exited, not killed */
/* BB = 0x7F means stopped */
#define EXIT(n) ((n & 0xFF) << 8)
#define KILL(n) (n & 0xFF)

int waitstatus = 0;

/* pid < -1: child waiting to be reaped
   pid = -1: child has been reaped and marked dead in waitpids()
   pid = 0: ignore this child */

int waitpid(int ignored, int* status, int flags)
{
	struct initrec *p, **pp;

	for(pp = cfg->inittab; (p = *pp); pp++)
		if(p->pid < -1) {
			int ret = -p->pid;
			*status = waitstatus;
			p->pid = ret;
			return ret;
		}

	return -1;
}

#define BUF 1000
char warnbuf[BUF+1];

void warn(const char* fmt, ...)
{
	va_list ap;
	char* p = warnbuf + strlen(warnbuf);

	va_start(ap, fmt);
	int n = vsnprintf(p, BUF, fmt, ap);
	va_end(ap);

	p[n] = '\0';
}

void reset(void)
{
	struct initrec *p, **pp;
	int i;

	memset(warnbuf, 0, BUF);

	for(i = 0, pp = cfg->inittab; (p = *pp); pp++, i++)
		p->pid = 11*(i+1);
};

extern void waitpids(void);

void run(struct initrec* p, int flags, int status)
{
	reset();
	waitstatus = status;
	p->pid = -p->pid;
	p->flags = flags;
	waitpids();
}

int main(void)
{
	/* Dry run */
	reset();
	waitpids();
	ASSERT(I1.pid > 0);
	ASSERT(I2.pid > 0);
	ASSERT(I3.pid > 0);
	ASSERT(I4.pid > 0);
	ASSERT(I5.pid > 0);
	ASSERT(I6.pid > 0);

	/* Simple common case, I1 and I3 die at the same time.
	   Make sure they get marked, and nothing else is changed.
	   This is the only test for the loop, the rest are one pid at a time. */
	reset();
	I1.pid = -I1.pid;
	I3.pid = -I3.pid;
	waitstatus = EXIT(0);
	waitpids();
	ASSERT(I1.pid == -1);
	ASSERT(I2.pid > 0);
	ASSERT(I3.pid == -1);
	ASSERT(I4.pid > 0);
	ASSERT(I5.pid > 0);
	ASSERT(I6.pid > 0);
	STREQUALS(warnbuf, "");

	/* Are signal flags removed properly? */
	run(&I1, C_FAST | P_SIGTERM, KILL(SIGTERM));
	ASSERT(I1.pid == -1);
	ASSERT(I1.flags == C_FAST);
	STREQUALS(warnbuf, "");

	/* Abnormal exit should be reported */
	run(&I3, C_FAST, EXIT(5));
	ASSERT(I3.pid == -1);
	ASSERT(I3.flags == C_FAST);
	STREQUALS(warnbuf, "[33] abnormal exit 5");

	/* Unexpected signal */
	run(&I5, C_FAST, KILL(SIGABRT));
	ASSERT(I5.pid == -1);
	ASSERT(I5.flags == C_FAST);
	STREQUALS(warnbuf, "[55] killed by SIGABRT");

	/* Unknown signal */
	run(&I4, C_FAST, KILL(100));
	ASSERT(I4.pid == -1);
	ASSERT(I4.flags == C_FAST);
	STREQUALS(warnbuf, "[44] killed by signal 100");

	/* Signal kills are ignored for entries with P_SIG*,
	   even if it's not the expected signal. */
	run(&I2, C_FAST | P_SIGTERM | P_SIGKILL, KILL(SIGBUS));
	ASSERT(I2.pid == -1);
	ASSERT(I2.flags == C_FAST);
	STREQUALS(warnbuf, "");

	/* C_HUSH entries are not reported */
	run(&I4, C_FAST | C_HUSH, KILL(SIGSEGV));
	ASSERT(I4.pid == -1);
	ASSERT(I4.flags == (C_FAST | C_HUSH));
	STREQUALS(warnbuf, "");

	return 0;
}
