#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

#define RET 0xAB

struct memblock newblock;	/* to keep the linker happy */
struct memblock scratch;

struct {
	int called;
	char* name;
	char* rlvls;
	char* flags;
	char* cmd;
	int exe;
} U;

extern int parseinitline(struct fileblock* fb, int strict);
int readinitdir(char* dir, int strict)
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

int scratchstring(char listcode, const char* string)
{
	return -1;
}

void test(input, name, rlvls, flags, cmd)
	const char *input;
	const char *name, *rlvls, *flags, *cmd;
{
	char* data = strdup(input);
	struct fileblock fb = {
		.name = "(none)",
		.line = 1,
		.buf = data,
		.ls = data,
		.le = data + strlen(data)
	};

	memset(&U, 0, sizeof(U));

	A(parseinitline(&fb, 0) == RET);

	A(U.called == 1);
	S(U.name, name);
	S(U.rlvls, rlvls);
	S(U.flags, flags);
	S(U.cmd, cmd);
	A(U.exe == 0);

	free(data);
}

int main(void)
{
	/* generic line */
	test("echo:123:wait,log:/bin/echo -n foo",
		"echo", "123", "wait,log", "/bin/echo -n foo");

	/* no name */
	test(":123:wait,log:/bin/test",
		"", "123", "wait,log", "/bin/test");

	/* no rlvls */
	test("test::wait,log:/bin/test",
		"test", "", "wait,log", "/bin/test");

	/* no flags */
	test("test:123::/bin/test",
		"test", "123", "", "/bin/test");

	/* sh */
	test("test:123:wait:!/bin/test",
		"test", "123", "wait", "!/bin/test");

	return 0;
}
