#include "_test.h"

extern int itostr(char* buf, size_t len, size_t n);

int main(void)
{
	char buf[100];
	int ret;

	/* One digit */
	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, 0);
	ASSERT(ret == 1);
	STREQUALS(buf, "0bcdef");

	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, 1);
	ASSERT(ret == 1);
	STREQUALS(buf, "1bcdef");

	/* Several digits */
	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, 116);
	ASSERT(ret == 3);
	STREQUALS(buf, "116def");

	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, -15);
	ASSERT(ret == 3);
	STREQUALS(buf, "-15def");

	/* Overflow */
	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, 1234);
	ASSERT(ret == 3);
	STREQUALS(buf, "123def");

	strcpy(buf, "abcdef");
	ret = itostr(buf, 2, -15);
	ASSERT(ret == 2);
	STREQUALS(buf, "-1cdef");

	return 0;
}
