/* gcc does sometimes "optimize" printf calls by replacing them with putchar or puts */

#include <sys/write.h>

int putchar(int c)
{
	char s = (char) c;
	return syswrite(1, &s, 1);
}
