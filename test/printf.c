#include <unistd.h>
#include <fcntl.h>
#include "_test.h"

/* We are testing vsnprintf whose paired snprintf is not used in init,
   may not be available, or (even worse) may come from unrelated libc. */

/* Beware of %m below: without sys_err_*, we are using full syserrlist
   from libtest! */

int wrap(char* buf, int len, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = vsnprintf(buf, len, fmt, ap);
	va_end(ap);
	return ret;
}

#define TEST(expected, ...) { \
	ret = wrap(data, DATA, __VA_ARGS__); \
	STREQUALS(data, expected); \
	INTEQUALS(ret, strlen(data)); \
}

#define DATA 1000
char data[DATA];

int main(void)
{
	int ret;

	TEST("here comes string",
	     "here %s string", "comes");
	TEST("two strings: one and another end",
	     "two strings: %s and %s end", "one", "another");
	TEST("a string and a number 123",
	     "a %s and a number %i", "string", 123);

	/* handling NULLs */
	TEST("string: (null)",
	     "string: %s", NULL);

	/* unterminated sequence at the end of string */
	TEST("bad case ",
	     "bad case %");

	/* unimplemented sequence */
	TEST("bad ",
	     "bad %e case");

	/* check %m */
	/* this may very well fail if libc has something different for ENOENT */
	open("/nonexistant", O_RDONLY);
	TEST("error: No such file or directory",
	     "error: %m");
	
	/* padding */
	TEST("here >abc   < padded",
	     "here >%-*s< padded", 6, "abc");
	TEST("here >abc< padded",
	     "here >%-*s< padded", 0, "abc");
	TEST("here >12   < padded",
	     "here >%-*i< padded", 5, 12);

	return 0;
}
