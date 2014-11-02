/* gcc does sometimes "optimize" printf calls by replacing them with putchar or puts */

#include <unistd.h>

int putchar(int c)
{
	char s = (char) c;
	return write(1, &s, 1);
}
