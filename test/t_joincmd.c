#include <string.h>
#include "../init.h"
#include "test.h"

int nextlevel;
int currlevel;
struct config* cfg;

NOCALL(levelmatch);

extern void joincmd(char* buf, int len, char** argv);

void check(char** argv, int len, char* expected)
{
	char buf[100];	/* must be greater than any tested len */
	int rln;
	char* ptr = buf + 1;

	memset(buf, '-', 100);
	joincmd(ptr, len, argv);
	rln = strlen(ptr);
	A(rln <= len - 1);
	S(ptr, expected);
	A(ptr[rln+2] == '-');
	A(ptr[-1] == '-');
}

int main(void)
{
	char* argv[] = { "echo", "foo", "bar", NULL };

	//               01234567890123
	check(argv, 14, "echo foo bar");

	//               0123456789012
	check(argv, 13, "echo foo bar");

	//               012345678901
	check(argv, 12, "echo foo...");

	//               0123456789
	check(argv, 10, "echo f...");

	//               012345679
	check(argv,  9, "echo ...");

	/* corner cases, should not happen but who knows */
	//               012345679
	check(argv,  4, "...");
	check(argv,  1, "");

	argv[0] = NULL;
	check(argv,  4, "");

	argv[0] = NULL;
	check(argv,  1, "");

	return 0;
}
