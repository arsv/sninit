#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	char s[] = "foobar";
	int i = 1247;
	long l = 1234567890;
	unsigned x = 0x123ABC;
	unsigned long lx = 0x123456789A;

	printf("non-format string\n"); /* this is going to be puts(), thanks gcc */
	printf("str: %s\n", s);
	printf("int: %i\n", i);
	printf("long: %li\n", l);
	printf("hex: 0x%08X\n", x);
	printf("long hex: 0x%lX\n", lx);
	printf("pointer: 0x%p\n", argv);

	open("nonexistant", O_RDONLY);
	printf("error: %m\n");

	return 0;
}
