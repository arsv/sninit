#include <string.h>

char* strsep(char **stringp, const char *delim)
{
	char* orig = *stringp;
	char* p;
  	const char* q;

	if(!orig)
		goto out;

	for(p = orig; *p; p++)
		for(q = delim; *q; q++)
			if(*q == *p) {
				*p = '\0';
				*stringp = p + 1;
				goto out;
			}
	*stringp = NULL;

out:	
	return orig;
}
