#include <sys/types.h>
#include <string.h>

char* strpbrk(const char* s, const char *accept)
{
	const char* a;

	for(; *s; s++)
		for(a = accept; *a; a++)
			if(*s == *a)
				return (char*)s;

	return 0;
}
