#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "init.h"
#include "init_conf.h"

/* to keep the linker happy */
int currlevel;
int state;
struct config* cfg;
extern struct memblock newblock;

char* inittab = NULL;
extern int configure(int strict);

void die(const char* msg, ...);
int warn(const char* msg, ...);
char* quote(const char* val);

void dump_config(struct config* cfg);
void dump_inittab(const char* var, struct initrec* inittab);
void dump_envp(const char* var, char** list);

int main(int argc, char** argv)
{
	if(argc < 2) 
		die("Bad call\n");

	inittab = argv[1];
	if(configure(1))
		die("Failed to compile inittab\n");

	dump_config((struct config*)newblock.addr);

	return 0;
}

void dump_inittab(const char* base, struct initrec* head)
{
	struct initrec* rec;
	char** p;
	int i;

	for(rec = head, i = 0; rec; rec = rec->next, i++) {
		if(rec->next)
			printf("static struct initrec %s%i;\n", base, i+1);

		printf("static struct initrec %s%i = {\n", base, i);

		if(rec->next)
			printf("\t.next = &%s%i,\n", base, i+1);
		else
			printf("\t.next = NULL,\n");
		printf("\t.name = %s,\n", quote(rec->name));
		printf("\t.rlvl = %i,\n", rec->rlvl);
		printf("\t.flags = %i,\n", rec->flags);
		printf("\t.pid = %i,\n", rec->pid);
		printf("\t.lastrun = %li,\n", rec->lastrun);
		printf("\t.lastsig = %li,\n", rec->lastsig);
		printf("\t.argv = { ");
		for(p = rec->argv; *p; p++)
			// no quoting for now
			printf("\"%s\", ", *p);
		printf(" NULL }\n");
		printf("};\n\n");	
	}
}

void dump_config(struct config* cfg)
{
	/* Current init.h has #define NULL line thanks to musl,
	   so there's no need to include stdlib.h */
	printf("#include \"init.h\"\n\n");

	if(cfg->inittab)
		dump_inittab("irec", cfg->inittab);
	if(cfg->env)
		dump_envp("env", cfg->env);

	printf("static struct config builtin = {\n");
	printf("\t.slippery = %i,\n", cfg->slippery);
	printf("\t.inittab = %s,\n", cfg->inittab ? "&irec0" : "NULL");
	printf("\t.env = %s,\n", cfg->env ? "env" : "NULL");
	printf("\t.time_to_restart = %i,\n", cfg->time_to_restart);
	printf("\t.time_to_SIGKILL = %i,\n", cfg->time_to_SIGKILL);
	printf("\t.time_to_skip = %i,\n", cfg->time_to_skip);
	printf("\t.logdir = %s\n", quote(cfg->logdir));
	printf("};\n\n");

	printf("struct config* cfg = &builtin;\n");
}

void dump_envp(const char* var, char** list)
{
	char** p;
	printf("static char* %s[] = {\n", var);
	for(p = list; *p; p++)
		printf("\t%s,\n", quote(*p));
	printf("\tNULL\n");
	printf("};\n\n");
}

char* quote(const char* val)
{
	static char buf[1024];
	if(!val) return "NULL";
	snprintf(buf, 1024, "\"%s\"", val);
	return buf;
}

void die(const char* msg, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, msg);
	vsnprintf(buf, 256, msg, ap);
	va_end(ap);

	write(2, buf, strlen(buf));
	_exit(-1);
}

int warn(const char* msg, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, msg);
	vsnprintf(buf, 256, msg, ap);
	va_end(ap);

	write(2, buf, strlen(buf));

	return 0;
}
