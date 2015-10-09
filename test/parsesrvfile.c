#include "../init.h"
#include "../init_conf.h"
#include "../config.h"
#include "_test.h"

#define RET 0xAB

extern struct fblock fileblock;

#define NAME 50
#define RLVL 50
#define CMD 1000

struct {
	int called;
	const char* name;
	const char* rlvl;
	const char* cmd;
	int exe;
} U;

extern int parsesrvfile(char* fullname, char* basename);

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

#define TEST(i, r, c, x) { \
	char* file = "/etc/rc/foo"; \
	char* base = "foo"; \
	\
	memset(data, 0, DATA); \
	strcpy(data, i); \
	\
	fileblock.buf = data; \
	fileblock.len = strlen(data); \
	fileblock.line = 0; \
	fileblock.ls = NULL; \
	fileblock.le = NULL; \
	\
	memset(&U, 0, sizeof(U)); \
	ASSERT(parsesrvfile(file, base) == RET); \
	ASSERT(U.called == 1); \
	STREQUALS(U.name, base); \
	STREQUALS(U.rlvl, r); \
	STREQUALS(U.cmd, c); \
	ASSERT(U.exe == x); \
}

#define DATA 1000
char data[DATA];

int main(void)
{
	/* The following assumes SRDEFAULT is "S3+"! */

	LOG("non-shebang, runlevels, no flags");
	TEST(	"#:123\n"
		"/bin/echo -n foo\n",
		"S123", "/bin/echo -n foo", 0);

	LOG("non-shebang, shell, with flags");
	TEST(	"#:123a\n"
		"! echo -n foo\n",
		"S123a", "! echo -n foo", 0);

	LOG("non-shebang, runlevels followed by comments");
	TEST(	"#:123\n"
		"# something goes here\n"
		"\n"
		"# one more comment line\n"
		"\n"
		"/bin/echo -n foo\n",

		"S123", "/bin/echo -n foo", 0);

	LOG("shebang, runlevels, flags");
	TEST(	"#!/bin/sh\n"
		"#:123\n"
		"echo -n foo\n",
		"S123", "/etc/rc/foo", 1);

	LOG("non-shebang, with comments");
	TEST(	"# some comment goes here\n"
		"/bin/echo -n foo\n",
		SRDEFAULT, "/bin/echo -n foo", 0);

	LOG("non-shebang, with comments, empty line after");
	TEST(	"# some comment goes here\n"
		"\n"
		"/bin/echo -n foo\n",
		SRDEFAULT, "/bin/echo -n foo", 0);

	LOG("non-shebang, with comments, empty line in-between");
	TEST(	"# some comment goes here\n"
		"  \n"
		"# some more comments\n"
		"\n"
		"/bin/echo -n foo\n",
		SRDEFAULT, "/bin/echo -n foo", 0);

	return 0;
}
