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
	char* mode;
	char* name;
	char* cmd;
	int exe;
} U;

extern int parseinitline(struct fileblock* fb, int strict);
int readinitdir(char* dir, int strict)
{
	return -1;
}

/*char* strdupnull(const char* str)
{
	return str ? strdup(str) : NULL;
}*/

int addinitrec(struct fileblock* fb, char* mode, char* name, char* cmd, int exe)
{
	U.called++;
	U.mode = mode;
	U.name = name;
	U.cmd = cmd;
	U.exe = exe;
	return RET;
}

int addenviron(const char* env)
{
	return 0;
}

int scratchenv(const char* string)
{
	return -1;
}

int setrunlevels(struct fileblock* fb, unsigned short* rlvl, char* runlevels)
{
	return -1;
}

void test(input, mode, name, cmd)
	const char *input;
	const char *name, *mode, *cmd;
{
	char* data = alloca(strlen(input) + 1);
	strcpy(data, input);
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
	S(U.mode, mode);
	S(U.name, name);
	S(U.cmd, cmd);
	A(U.exe == 0);
}

/* This whole test is a remnant of the times when initline format
   actually required testing. At present there is little to test. */
int main(void)
{
	/* generic line */
	test("W123\techo\t/bin/echo -n foo",
		"W123", "echo", "/bin/echo -n foo");

	/* arbitrary spaces */
	test("W123  echo \t /bin/echo -n foo",
		"W123", "echo", "/bin/echo -n foo");

	/* sh */
	test("W123  test  !/bin/test",
		"W123", "test", "!/bin/test");

	return 0;
}
