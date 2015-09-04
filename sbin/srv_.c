#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "srv_.h"

extern const char* tag;
extern const int lifetime;

#define BUF 1024
static char saybuf[BUF];

/* this should have been printf, but printf from libtest is not one-write
   (which results in messy output) and bringing in ../sys_printf.c is kinda
   messy by itself. So why bother, it's not like this is something important. */

void say(const char* what)
{
	strcpy(saybuf, tag);
	strcat(saybuf, ": ");
	strcat(saybuf, what);
	strcat(saybuf, "\n");
	write(1, saybuf, strlen(saybuf));
}

void sighandler(int sig)
{
	switch(sig) {
		case SIGKILL: say("dying on SIGKILL"); _exit(0); break;
		case SIGINT:  say("dying on SIGINT");  _exit(0); break;
		case SIGTERM: say("dying on SIGTERM"); _exit(0); break;
		case SIGHUP:  say("got SIGHUP"); break;
		default: say("ignoring unexpected signal"); break;
	}
}

void trapsig(void)
{
	struct sigaction sa = { .sa_handler = sighandler, .sa_flags = SA_RESTART };
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP,  &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
}

void sleepx(int sec)
{
	struct timespec ts = { .tv_sec = sec, .tv_nsec = 0 };
	struct timespec tr;
	while(1) {
		if(!nanosleep(&ts, &tr))
			break;
		if(tr.tv_sec <= 0)
			break;
		ts = tr;
	};
}
