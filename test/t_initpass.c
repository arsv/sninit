#include <unistd.h>
#include <signal.h>
#include "test.h"
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
		fprintf(stderr, "log overflow\n");
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
	p->pid = 1;
}

void stop(struct initrec* p)
{
	mark('-', p->name);
	p->pid = -1;
}

void died(struct initrec* p)
{
	p->pid = -1;
}

#define NOCALL(f) void f() { nocall(#f); }
int nocall(const char* func)
{
	fprintf(stderr, "called %s()\n", func);
	kill(getpid(), SIGTERM);
	return 0;
}
NOCALL(sexec);
