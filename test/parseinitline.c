#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

#define RET 0xAB

struct {
	int called;
	const char* name;
	const char* rlvl;
	const char* cmd;
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

int addinitrec(const char* name, const char* rlvl, char* cmd, int exe)
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

#define TEST(i, n, r, c) { \
	memset(data, 0, DATA); \
	strcpy(data, i); \
	memset(&U, 0, sizeof(U)); \
	ASSERT(parseinitline(data) == RET); \
	ASSERT(U.called == 1); \
	STREQUALS(U.name, n); \
	STREQUALS(U.rlvl, r); \
	STREQUALS(U.cmd, c); \
	ASSERT(U.exe == 0); \
}

#define DATA 1000
char data[DATA];

/* This whole test is a remnant of the times when initline format
   actually required testing. At present there is little to test. */
int main(void)
{
	/* generic line */
	TEST("echo\tW123\t/bin/echo -n foo",
		"echo", "W123", "/bin/echo -n foo");

	/* arbitrary spaces */
	TEST("echo  W123 \t /bin/echo -n foo",
		"echo", "W123", "/bin/echo -n foo");

	/* sh */
	TEST("W123  test  !/bin/test",
		"W123", "test", "!/bin/test");

	return 0;
}
