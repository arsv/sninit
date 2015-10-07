#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "../config.h"
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

/* Run (almost) full config sequence and check whether newblock is being formed properly */

int currlevel = 0;
struct config* cfg = NULL;
extern struct newblock nb;

extern void initcfgblocks(void);
extern int finishinittab(void);
extern int parseinitline(char* line, int strict);
extern void rewirepointers(void);
extern int mmapblock(int size);

void die(const char* fmt, ...) __attribute__((format(printf, 1, 2))) __attribute__((noreturn));

#define HEAP 1024
char heap[HEAP];

int levelmatch(struct initrec* p, int level)
{
	return 0;
}

struct initrec* findentry(const char* name)
{
	return NULL;
}

int parseinitline_(char* testline)
{
	char* line = strncpy(heap, testline, HEAP);
	return parseinitline(line, 1);
}

int checkptr(void* ptr)
{
	return (ptr >= nb.addr && ptr < nb.addr + nb.len);
}

#define blockoffset(ptr) ( (int)((void*)(ptr) - nb.addr) )

void dumpenv(struct config* cfg)
{
	char** p;

	printf("ENV: %p [%i]\n", cfg->env, blockoffset(cfg->env));
	for(p = cfg->env; p && *p; p++) {
		if(!checkptr(*p))
			die("ENV %p bad pointer\n", p);
		printf("  [%i] -> [%i] %s\n",
				blockoffset(p),
				blockoffset(*p),
				*p ? *p : "NULL");
	}
}

void dumptab(struct config* cfg)
{
	struct initrec *q, **qq;
	char** p;
	int i;

	printf("TAB: %p [%i]\n", cfg->inittab, blockoffset(cfg->inittab));
	for(qq = cfg->inittab; (q = *qq); qq++) {
		printf("  [%i] name=\"%s\" rlvl=%i flags=%i\n",
				blockoffset(q), q->name, q->rlvl, q->flags);
		for(i = 0, p = q->argv; p && *p; p++, i++)
			if(checkptr(*p))
				printf("\targv[%i] -> [%i] \"%s\"\n", i, blockoffset(*p), *p);
			else
				printf("\targv[%i] BAD %p\n", i, *p);
	}
}

void dumpconfig(void)
{
	struct config* cfg = NCF;

	A(checkptr(cfg));
	printf("NCF: %p [%i]\n", cfg, blockoffset(cfg));

	dumptab(cfg);
	dumpenv(cfg);
}

int main(void)
{
	T(mmapblock(sizeof(struct config) + sizeof(struct scratch)));

	T(parseinitline_("# comment here"));
	T(parseinitline_(""));
	T(parseinitline_("FOO=something"));
	T(parseinitline_("PATH=/bin:/sbin:/usr/bin"));
	T(parseinitline_(""));
	T(parseinitline_("time    W12345   /sbin/hwclock -s"));
	T(parseinitline_("mount   W12345   /bin/mount -a"));

	T(finishinittab())
	rewirepointers();

	dumpconfig();
	
	return 0;
}

void die(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	kill(getpid(), SIGUSR1);
	_exit(-1);
};
