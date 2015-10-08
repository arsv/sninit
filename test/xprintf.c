#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	char s[] = "foobar";
	int i = 1247;
	unsigned x = 0x123ABC;
#if __WORDSIZE == 64
	long l = 1234567890;
	unsigned long lx = 0x123456789ABCDE;
	void* ptr = (void*)0x1122334455667788;
#else
	long l = 12345678;
	unsigned long lx = 0x12345678;
	void* ptr = (void*)0x11223344;
#endif

	printf("non-format string\n"); /* this is going to be puts(), thanks gcc */
	printf("str: %s\n", s);
	printf("int: %i\n", i);
	printf("long: %li\n", l);
	printf("hex: 0x%08X\n", x);
	printf("long hex: 0x%lX\n", lx);
	printf("pointer: 0x%p\n", ptr);

	open("nonexistant", O_RDONLY);
	printf("error: %m\n");

	return 0;
}
