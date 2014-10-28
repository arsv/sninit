/* gcc does sometimes "optimize" printf calls by replacing them with putchar or puts */

#include <unistd.h>
#include <string.h>

int puts(char* s)
{
	write(1, s, strlen(s));
	write(1, "\n", 1);
	return 1;
}
