#include <string.h>

char *strncpy(char* d, const char* s, size_t n)
{
	const char* e = s + n;
	char* ret = d;

	while(*s && s < e)
		*(d++) = *(s++);
	*d = '\0';

	return ret;
}
