#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

#define RET 0xAB

//struct memblock newblock;	/* to keep the linker happy */
struct memblock scratch;

struct {
	int called;
	char* name;
	char* rlvls;
	char* flags;
	char* cmd;
	int exe;
} U;

extern int parsesrvfile(struct fileblock* fb, char* basename, int diri);
extern int mmapblock(struct memblock* m, int size);

int scratchstring(char listcode, const char* string)
{
	return -1;
}

char* strdupnull(const char* str)
{
	return str ? strdup(str) : NULL;
}

int addinitrec(struct fileblock* fb, char* name, int diri, char* rlvls, char* flags, char* cmd, int exe)
{
	U.called++;
	U.name = strdupnull(name);
	U.rlvls = strdupnull(rlvls);
	U.flags = strdupnull(flags);
	U.cmd = strdupnull(cmd);
	U.exe = exe;
	return RET;
}

void test(input, rlvls, flags, cmd, exe)
	const char *input;
	const char *rlvls, *flags, *cmd;
	int exe;
{
	char* data = strdup(input);
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
	A(parsesrvfile(&fb, base, -1) == RET);
	A(U.called == 1);
	S(U.name, base);
	S(U.rlvls, rlvls);
	S(U.flags, flags);
	S(U.cmd, cmd);
	A(U.exe == exe);
	free(data);
}

int main(void)
{
	T(mmapblock(&scratch, sizeof(struct scratch)));
	memset(scratch.addr, sizeof(struct scratch), 0);

	/* one-liner */
	test(	"#:123:wait,log:/bin/echo -n foo\n",
		"123", "wait,log", "/bin/echo -n foo\n", 0);

	/* one-liner, sh */
	test(	"#:123:wait,log:!echo -n foo\n",
		"123", "wait,log", "!echo -n foo\n", 0);

	/* multi-line */
	test(	"#:123:wait,log\n"
		"/bin/echo -n foo\n",
		"123", "wait,log", "/bin/echo -n foo\n", 0);

	/* multi-line, sh */
	test(	"#:123:wait,log:!\n"
		"echo -n foo\n",
		"123", "wait,log", "!\necho -n foo\n", 0);

	/* shebang */
	test(	"#!/bin/sh\n"
		"#:123:wait,log\n"
		"echo -n foo\n",

		"123", "wait,log", "/etc/rc/foo", 1);

	return 0;
}
