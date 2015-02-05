#include <sys/types.h>
#include <string.h>

int memcmp(const void *dst, const void *src, size_t count)
{
	int r;
	const unsigned char* d = dst;
	const unsigned char* s = src;

	for(; count--; d++, s++)
		if(unlikely(r = (*d - *s)))
			return r;

	return 0;
}
