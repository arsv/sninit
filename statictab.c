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

extern char* inittab;
extern int configure(int strict);

void die(const char* msg, ...);
int warn(const char* msg, ...);
char* quote(const char* val);

void dump_config(struct config* cfg);
void dump_tab(const char* var, struct initrec** inittab);
void dump_env(const char* var, char** list);
void dump_rec(const char* var, int i, struct initrec* p);

int main(int argc, char** argv)
{
	if(argc < 2) 
		die("Bad call\n");

	inittab = argv[1];
	if(configure(STRICT))
		die("Failed to compile inittab\n");

	dump_config((struct config*)newblock.addr);

	return 0;
}

void dump_config(struct config* cfg)
{
	printf("#include \"init.h\"\n\n");
	printf("#define NULL ((void*)0)\n");	/* eh */

	dump_tab("builtin_tab", cfg->inittab);
	dump_env("builtin_env", cfg->env);

	printf("static struct config builtin = {\n");
	printf("\t.inittab = builtin_tab + 1,\n");
	printf("\t.initnum = %i,\n", cfg->initnum);
	printf("\t.env = builtin_env,\n");
	printf("};\n\n");

	printf("struct config* cfg = &builtin;\n");
}

void dump_env(const char* var, char** list)
{
	char** p;
	printf("static char* %s[] = {\n", var);
	for(p = list; *p; p++)
		printf("\t%s,\n", quote(*p));
	printf("\tNULL\n");
	printf("};\n\n");
}

void dump_tab(const char* var, struct initrec** inittab)
{
	const char* rec = "irec";
	struct initrec *p, **pp;
	int i, n;

	for(i = 0, pp = inittab; (p = *pp); pp++, i++)
		dump_rec(rec, i, p);
	n = i;

	printf("static struct initrec* %s[] = {\n", var);
		printf("\tNULL,\n");
	for(i = 0; i < n; i++)
		printf("\t&%s%i,\n", rec, i);
		printf("\tNULL\n");
	printf("};\n\n");
}

void dump_rec(const char* var, int i, struct initrec* p)
{
	char** a;

	printf("static struct initrec %s%i = {\n", var, i);

	printf("\t.name = %s,\n", quote(p->name));
	printf("\t.rlvl = %i,\n", p->rlvl);
	printf("\t.flags = %i,\n", p->flags);
	printf("\t.pid = %i,\n", p->pid);
	printf("\t.lastrun = %li,\n", (long)p->lastrun);
	printf("\t.lastsig = %li,\n", (long)p->lastsig);
	printf("\t.argv = { ");
	for(a = p->argv; *a; a++)
		// no quoting for now
		printf("\"%s\", ", *a);
		printf(" NULL }\n");

	printf("};\n");
}

char* quote(const char* val)
{
	static char buf[1024];
	int len = strlen(val);
	buf[0] = '"';
	strncpy(buf + 1, val, 1000);
	buf[len+1] = '"';
	buf[len+2] = '\0';
	return buf;
}

void die(const char* msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vprintf(msg, ap);
	va_end(ap);

	_exit(-1);
}

int warn(const char* msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vprintf(msg, ap);
	va_end(ap);

	return 0;
}
