#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "_test.h"
#include "../init.h"

int state;
int currlevel;
int nextlevel;
int sublevels;

/* not used, for linking only */
int timetowait;
int initctlfd;

#define BUF 1024
char passlog[BUF];
int passleft;
int passptr;

void reset(void)
{
	passlog[0] = '\0';
	passleft = BUF;
	passptr = 0;
}

void mark(char what, const char* name)
{
	int len = strlen(name);
	int ptr = passptr;
	if(passleft < len + 3) {
		warn("log overflow");
		return;
	}	
	passlog[passptr++] = what;
	strncpy(passlog + passptr, name, len); passptr += len;
	passlog[passptr] = '\0';
	passleft -= (passptr - ptr);
}

void spawn(struct initrec* p)
{
	mark('+', p->name);
	p->flags &= ~P_SIGTERM;
	p->pid = 1;
}

void stop(struct initrec* p)
{
	mark('-', p->name);
	p->flags |= P_SIGTERM;
}

void died(struct initrec* p)
{
	p->pid = -1;
	p->flags &= ~P_SIGTERM;
}

void killed(struct initrec* p)
{
	if(!(p->flags & P_SIGTERM))
		warn("FAIL: %s has not been sent a signal", p->name);
	p->pid = -1;
	p->flags &= ~P_SIGTERM;
}

NOCALL(execinitrec);
