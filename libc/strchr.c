#include <string.h>
#include <sys/cdefs.h>

char *strchr(register const char *t, int c)
{
	register char ch = c;

    	while(likely(*t != ch))
		if (unlikely(!*t))
			return 0;
		else
			++t;

	return (char*)t;
}
