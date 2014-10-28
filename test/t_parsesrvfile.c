#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

#define RET 0xAB

//struct memblock newblock;	/* to keep the linker happy */

struct {
	int called;
	char* name;
	char* rlvls;
	char* flags;
	char* cmd;
	int exe;
} U;

extern int parsesrvfile(struct fileblock* fb, char* basename);
extern int mmapblock(struct memblock* m, int size);

int addinitrec(struct fileblock* fb, char* name, char* runlevels, char* flags, char* cmd, int exe)
{
	U.called++;
	U.name = heapdupnull(name);
	U.rlvls = heapdupnull(runlevels);
	U.flags = heapdupnull(flags);
	U.cmd = heapdupnull(cmd);
	U.exe = exe;
	return RET;
}

int addenviron(const char* env)
{
	return 0;
}

void test(input, rlvls, flags, cmd, exe)
	const char *input;
	const char *rlvls, *flags, *cmd;
	int exe;
{
	char* data = heapdup(input);
	char* file = "/etc/rc/foo";
	char* base = "foo";
	struct fileblock fb = {
		.name = file,
		.line = 0,
		.buf = data,
		.len = strlen(data),
		.ls = NULL,
		.le = NULL
	};

	memset(&U, 0, sizeof(U));
	A(parsesrvfile(&fb, base) == RET);
	A(U.called == 1);
	S(U.name, base);
	S(U.rlvls, rlvls);
	S(U.flags, flags);
	S(U.cmd, cmd);
	A(U.exe == exe);
}

int main(void)
{
	L("non-shebang, with flags");
	test(	"#:123:wait,log\n"
		"/bin/echo -n foo\n",
		"123", "wait,log", "/bin/echo -n foo", 0);

	L("non-shebang, shell, with flags");
	test(	"#:123:wait,log\n"
		"! echo -n foo\n",
		"123", "wait,log", "! echo -n foo", 0);

	L("non-shebang, flags but no runlevels");
	test(	"#::wait,log\n"
		"/bin/echo -n foo\n",
		"", "wait,log", "/bin/echo -n foo", 0);

	L("non-shebang, runlevels w/o flags");
	test(	"#:123\n"
		"/bin/echo -n foo\n",
		"123", NULL, "/bin/echo -n foo", 0);

	L("non-shebang, runlevels followed by comments");
	test(	"#:123\n"
		"# something goes here\n"
		"\n"
		"# one more comment line\n"
		"\n"
		"/bin/echo -n foo\n",

		"123", NULL, "/bin/echo -n foo", 0);

	L("shebang, runlevels, flags");
	test(	"#!/bin/sh\n"
		"#:123:wait,log\n"
		"echo -n foo\n",

		"123", "wait,log", "/etc/rc/foo", 1);

	/* removed! */

	//L("non-shebang, no #: line");
	//test(	"/bin/echo -n foo\n",
	//	NULL, NULL, "/bin/echo -n foo", 0);
	
	//L("non-shebang, comments but no #: line");
	//test(	"# some comment goes here\n"
	//	"\n"
	//	"/bin/echo -n foo\n",
	//	NULL, NULL, "/bin/echo -n foo", 0);

	//L("shebang, no #: line");
	//test(	"#!/bin/sh\n"
	//	"echo -n foo\n",
	//	NULL, NULL, "/etc/rc/foo", 1);

	return 0;
}
