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
	char* rlvl;
	char* cmd;
	int exe;
} U;

extern int parsesrvfile(struct fileblock* fb, char* basename);
extern int mmapblock(struct memblock* m, int size);

int addinitrec(struct fileblock* fb, char* name, char* rlvl, char* cmd, int exe)
{
	U.called++;
	U.name = heapdupnull(name);
	U.rlvl = heapdupnull(rlvl);
	U.cmd = heapdupnull(cmd);
	U.exe = exe;
	return RET;
}

int addenviron(const char* env)
{
	return 0;
}

void test(input, rlvl, cmd, exe)
	const char *input;
	const char *rlvl, *cmd;
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
	S(U.rlvl, rlvl);
	S(U.cmd, cmd);
	A(U.exe == exe);
}

int main(void)
{
	L("non-shebang, runlevels, no flags");
	test(	"#:123\n"
		"/bin/echo -n foo\n",
		":123", "/bin/echo -n foo", 0);

	L("non-shebang, shell, with flags");
	test(	"#:123l\n"
		"! echo -n foo\n",
		":123l", "! echo -n foo", 0);

	L("non-shebang, runlevels followed by comments");
	test(	"#:123\n"
		"# something goes here\n"
		"\n"
		"# one more comment line\n"
		"\n"
		"/bin/echo -n foo\n",

		":123", "/bin/echo -n foo", 0);

	L("shebang, runlevels, flags");
	test(	"#!/bin/sh\n"
		"#:123\n"
		"echo -n foo\n",
		":123", "/etc/rc/foo", 1);

	L("non-shebang, with comments");
	test(	"# some comment goes here\n"
		"/bin/echo -n foo\n",
		"", "/bin/echo -n foo", 0);

	L("non-shebang, with comments, empty line after");
	test(	"# some comment goes here\n"
		"\n"
		"/bin/echo -n foo\n",
		"", "/bin/echo -n foo", 0);

	L("non-shebang, with comments, empty line in-between");
	test(	"# some comment goes here\n"
		"  \n"
		"# some more comments\n"
		"\n"
		"/bin/echo -n foo\n",
		"", "/bin/echo -n foo", 0);

	return 0;
}
