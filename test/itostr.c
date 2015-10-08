#include "_test.h"

extern int itostr(char* buf, size_t len, size_t n);

int main(void)
{
	char buf[100];
	int ret;

	/* One digit */
	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, 0);
	A(ret == 1);
	S(buf, "0bcdef");

	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, 1);
	A(ret == 1);
	S(buf, "1bcdef");

	/* Several digits */
	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, 116);
	A(ret == 3);
	S(buf, "116def");

	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, -15);
	A(ret == 3);
	S(buf, "-15def");

	/* Overflow */
	strcpy(buf, "abcdef");
	ret = itostr(buf, 3, 1234);
	A(ret == 3);
	S(buf, "123def");

	strcpy(buf, "abcdef");
	ret = itostr(buf, 2, -15);
	A(ret == 2);
	S(buf, "-1cdef");

	return 0;
}
