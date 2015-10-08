#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

#define RET 0xAB

struct {
	int called;
	char* name;
	char* rlvl;
	char* cmd;
	int exe;
} U;

extern int parseinitline(char* l);

int readinitdir(char* dir, int strict)
{
	return -1;
}

/*char* strdupnull(const char* str)
{
	return str ? strdup(str) : NULL;
}*/

int addinitrec(char* name, char* rlvl, char* cmd, int exe)
{
	U.called++;
	U.name = name;
	U.rlvl = rlvl;
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

int setrunlevels(unsigned short* rlvl, char* runlevels)
{
	return -1;
}

void test(input, name, rlvl, cmd)
	const char *input;
	const char *name, *rlvl, *cmd;
{
	char* data = alloca(strlen(input) + 1);
	strcpy(data, input);

	memset(&U, 0, sizeof(U));

	A(parseinitline(data) == RET);

	A(U.called == 1);
	S(U.name, name);
	S(U.rlvl, rlvl);
	S(U.cmd, cmd);
	A(U.exe == 0);
}

/* This whole test is a remnant of the times when initline format
   actually required testing. At present there is little to test. */
int main(void)
{
	/* generic line */
	test("echo\tW123\t/bin/echo -n foo",
		"echo", "W123", "/bin/echo -n foo");

	/* arbitrary spaces */
	test("echo  W123 \t /bin/echo -n foo",
		"echo", "W123", "/bin/echo -n foo");

	/* sh */
	test("W123  test  !/bin/test",
		"W123", "test", "!/bin/test");

	return 0;
}
