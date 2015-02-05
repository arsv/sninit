#include <string.h>

void* memcpy(void* dst, const void* src, size_t n)
{
	void* r = dst;
	char* d = dst;
	const char* s = src;

	while(n--) *(d++) = *(s++);

	return r;
}
