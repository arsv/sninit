#include <string.h>
#include "test.h"

int mkreldirname(char* buf, int len, const char* base, const char* dir);

/* To keep linker happy. No-one will call them anyway. */
void scratchstring(void) { };
void mmapfile(void) { };
void munmapfile(void) { };
void nextline(void) { };
void addinitrec(void) { };

int main(void)
{
	/* generic case */
	//              012345678901234";
	char buf[26] = "abcdefghijk+---";
	char* base   = "/foo/bar";
	char* dir =         "baz/rc";
	int r;

	r = mkreldirname(buf, 15, base, dir);
	A(r == 12);
	S(buf, "/foo/baz/rc/");
	A(buf[13] == '-');
	A(buf[14] == '-');

	/* off-by-one */
	//            012345678901234
	strncpy(buf, "abcdefghijk+---", 15);
	r = mkreldirname(buf, 12, base, dir);
	A(r < 0);
	A(buf[12] == '-');
	A(buf[13] == '-');

	/* absolute dir */
	//            012345678901234
	strncpy(buf, "abcdefgh+------", 15);
	char* abs =  "/etc/rc/";
	r = mkreldirname(buf, 12, base, abs);
	A(r > 0);
	S(buf, abs);
	A(buf[9] == '-');

	/* off-by-one in abs dir */
	//            012345678901234
	strncpy(buf, "abcdefg+-------", 15);
	r = mkreldirname(buf, 7, base, abs);
	A(r < 0);
	A(buf[7] == '+');

	return 0;
}
