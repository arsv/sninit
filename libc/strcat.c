#include <string.h>

char* strcat(char* s, const char* t)
{
	char* r = s;

	for(s += strlen(s); (*s = *t); s++, t++)
		;

	return r;
}
