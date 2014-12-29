#include <stdlib.h>
#include <string.h>

char* getenv(const char *s)
{
	unsigned int len;
	char** e;

	if(!environ || !s)
		return 0;

	len = strlen(s);
	for(e = environ; *e; e++)
		if((memcmp(*e, s, len) == 0) && ((*e)[len] == '='))
			return (*e) + len + 1;

	return NULL;
}
