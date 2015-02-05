#include <sys/types.h>
#include <string.h>

void* memset(void* dst, int s, size_t count)
{
	char* a = dst;

	while(count--)
		*(a++) = s;

	return dst;
}
