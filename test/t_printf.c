#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "test.h"

void test(const char* result, const char* fmt, ...)
{
	int r;
	int len = 1024;
	char buf[len];
	va_list ap;

	va_start(ap, fmt);
	r = vsnprintf(buf, len, fmt, ap);
	va_end(ap);

	int ret = strlen(result);
	S(buf, result);
	Ac(r == ret, "ret = %i", r);
}

int main(void)
{
	test("here comes string",
	     "here %s string", "comes");
	test("two strings: one and another end",
	     "two strings: %s and %s end", "one", "another");
	test("a string and a number 123",
	     "a %s and a number %i", "string", 123);

	/* handling NULLs */
	test("string: (null)",
	     "string: %s", NULL);

	/* unterminated sequence at the end of string */
	test("bad case ",
	     "bad case %");

	/* unimplemented sequence */
	test("bad ",
	     "bad %e case");

	/* check %m */
	/* this may very well fail if libc has something different for ENOENT */
	chdir("/nonexistant");
	test("error: No such file or directory",
	     "error: %m");


	return 0;
}
