#include <string.h>
#include "../init.h"
#include "test.h"

int nextlevel;
int currlevel;
struct config* cfg;

/* The function itself is trivial, *and* it is always called
   with a fixed-width, way-too-long buffer, so the tests are
   here just in case. */

extern void rlstr(char* str, int len, int mask);

void check(const char* file, int line, int len, const char* exp, int mask)
{
	int buflen = len + 10;
	char buf[buflen];

	memset(buf, '*', buflen);
	buf[buflen-1] = '\0';
	rlstr(buf, len, mask);

	if(strlen(buf) >= len)
		printf("%s:%i: FAILED overflow 1 (%i over %i)\n", file, line, (int)strlen(buf), len);
	else if(strcmp(buf, exp))
		printf("%s:%i: FAILED \"%s\" != \"%s\"\n", file, line, buf, exp);
	else if(buf[len] != '*')
		printf("%s:%i: FAILED overflow 2\n", file, line);
	else
		printf("%s:%i: OK \"%s\"\n", file, line, buf);
}

#define CHECK(len, res, runlevels) check(__FILE__, __LINE__, len, res, runlevels)

int main(void)
{
	/* Typical cases */
	CHECK(16, "1",       (1<<1));
	CHECK(16, "1ab",     (1<<1) | (1<<0xA) | (1<<0x0B));
	CHECK(16, "9abcdef", (1<<9) | (1<<0xA) | (1<<0x0B) | (1<<0x0C) | (1<<0x0D) | (1<<0x0E) | (1<<0x0F));
	CHECK(16, "0",       (1<<0));
	CHECK(16, "3af",     (1<<3) | (1<<0xA) | (1<<0xF));

	return 0;
}
