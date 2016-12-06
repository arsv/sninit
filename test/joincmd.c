#include "../init.h"
#include "_test.h"

int nextlevel;
int currlevel;
struct config* cfg;

#define CHECK(argv, len, expected) { \
	memset(buf, '-', 100); \
	joincmd(ptr, len, argv); \
	rln = strlen(ptr); \
	ASSERT(rln <= len - 1); \
	STREQUALS(ptr, expected); \
	ASSERT(ptr[rln+2] == '-'); \
	ASSERT(ptr[-1] == '-'); \
}

int main(void)
{
	char* argv[] = { "echo", "foo", "bar", NULL };

	char buf[100];	/* must be greater than any tested len */
	int rln;
	char* ptr = buf + 1;

	//               01234567890123
	CHECK(argv, 14, "echo foo bar");

	//               0123456789012
	CHECK(argv, 13, "echo foo bar");

	//               012345678901
	CHECK(argv, 12, "echo foo...");

	//               0123456789
	CHECK(argv, 10, "echo f...");

	//               012345679
	CHECK(argv,  9, "echo ...");

	/* corner cases, should not happen but who knows */
	//               012345679
	CHECK(argv,  4, "...");
	CHECK(argv,  1, "");

	argv[0] = NULL;
	CHECK(argv,  4, "");

	argv[0] = NULL;
	CHECK(argv,  1, "");

	return 0;
}
