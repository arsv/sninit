/* gcc does sometimes "optimize" printf calls by replacing them with putchar or puts */

#include <sys/write.h>
#include <string.h>

int puts(char* s)
{
	syswrite(1, s, strlen(s));
	syswrite(1, "\n", 1);
	return 1;
}
