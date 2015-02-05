#include <string.h>

void* memchr(const void* s, int c, size_t n)
{
	const unsigned char *p = (unsigned char *) s;

	for(; n--; p++)
		if(*p == c)
			return (void*)p;

	return 0;
}
