#include <string.h>

char* strcat(register char* s,register const char* t)
{
	char *dest = s;
	s += strlen(s);

	while((*s = *t)) {
		++s;
		++t;
	}

	return dest;
}
