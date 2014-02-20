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

int state;
struct config* cfg;
int currlevel;
char* inittab = INITTAB;
extern struct memblock newblock;

extern void initcfgblocks(void);
extern int finishenvp(void);
extern int parseinitline(struct fileblock* fb, int strict);
extern void rewirepointers(void);
extern int mmapblock(struct memblock* m, int size);

void die(const char* fmt, ...) __attribute__((format(printf, 1, 2))) __attribute__((noreturn));

int parseinitline_(char* testline)
{
	char* line = strdup(testline);
	struct fileblock fb = {
		.name="(none)",
		.line=1,
		.buf = line,
		.ls = line,
		.le = line + strlen(line)
	};
	int ret = parseinitline(&fb, 1);
	free(line);
	return ret;
}

int checkptr(struct memblock* block, void* ptr)
{
	//printf("checkptr({%p + 0x%x}, %p = 0x%lx)\n", block->addr, block->len, ptr, ptr - block->addr);
	return (ptr >= block->addr && ptr < block->addr + block->len);
}

void dump_env(struct memblock* block);
void dump_inittab(struct memblock* block);

void dump(struct memblock* block)
{
	struct config* cfg = (struct config*) block->addr;

	A(checkptr(block, cfg));
	//printf("initdefault = %i\n", cfg->initdefault);
	printf("slippery = 0x%04X\n", cfg->slippery);

	printf("time_to_restart = %i\n", cfg->time_to_restart);
	printf("time_to_SIGKILL = %i\n", cfg->time_to_SIGKILL);
	printf("time_to_skip = %i\n", cfg->time_to_skip);

	dump_env(block);
	dump_inittab(block);
}

void dump_env(struct memblock* block)
{
	struct config* cfg = (struct config*) block->addr;
	char** p;

	printf("ENV: %p [%li]\n", cfg->env, (void*)cfg->env - block->addr);
	for(p = cfg->env; p && *p; p++) {
		if(!checkptr(block, *p))
			die("ENV %p bad pointer\n", p);
		printf("%li = [%li] %s\n", (void*)p - block->addr, (void*)*p - block->addr, *p ? *p : "NULL");
	}
}

void dump_inittab(struct memblock* block)
{
	struct config* cfg = (struct config*) block->addr;
	struct initrec* q;
	char** p;
	int i;

	printf("INITTAB: %p\n", cfg->inittab);
	for(q = cfg->inittab; q; q = q->next) {
		printf("name=\"%s\" rlvl=0x%04x flags=0x%04x\n", q->name, q->rlvl, q->flags);
		for(i = 0, p = q->argv; p && *p; p++, i++)
			if(checkptr(block, *p))
				printf("\targv[%i]=\"%s\"\n", i, *p);
			else
				printf("\targv[%i] BAD %p\n", i, *p);
	}
}

int main(void)
{
	T(mmapblock(&newblock, IRALLOC + sizeof(struct config) + sizeof(struct scratch)));
	initcfgblocks();

	T(parseinitline_("# comment here"));
	T(parseinitline_(""));
	T(parseinitline_("FOO=something"));
	T(parseinitline_("PATH=/bin:/sbin:/usr/bin"));
	T(parseinitline_(""));
	T(parseinitline_("time:12345:wait:/sbin/hwclock -s"));
	T(parseinitline_("mount:12345:wait:/bin/mount -a"));

	T(finishenvp())
	rewirepointers();

	dump(&newblock);
	
	return 0;
}

void die(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	kill(getpid(), SIGUSR1);
	exit(-1);
};
