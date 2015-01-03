#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

/* Empty symbols to keep the linker happy */
struct memblock newblock;
void addstrargarray(void) { };
void addstringarray(void) { };
void mextendblock(void) { };
void checkdupname(void) { };
void scratchptr(void) { };
void addstring(void) { };

extern int setflags(struct fileblock* fb, struct initrec* entry, char* flagstring);

#define _(x) strcpy(buf, x)

#define test(str, val)\
	rec.flags = 0;\
	T(setflags(&fb, &rec, _(str)));\
	A(rec.flags == (val))

#define bad(str)\
	rec.flags = 0xFE7F;\
	T(!setflags(&fb, &rec, _(str)));\
	A(rec.flags == 0xFE7F)

int main(void)
{
	struct fileblock fb = { .name = "none", .line = 1 };
	struct initrec rec;
	char buf[100];

	test("", 0);

	test("respawn", 0);
	test("once", C_ONCE);
	test("wait", C_WAIT | C_ONCE);
	test("hold", C_WAIT);

	test("log", C_LOG);
	test("tty", C_TTY);
	test("abort", C_USEABRT);

	test("wait,null", C_WAIT | C_ONCE | C_NULL);
	test("abort,log", C_USEABRT | C_LOG);

	bad("what");
	bad("wait,");
	bad(",tty");

	return 0;
}
