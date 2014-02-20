#include <string.h>
#include <time.h>

/* Format current time into buf */
/* This is fallback implementation using libc calls;
   see sys_timestamp.c, sys_time_tz.c, sys_time_notz.c */

int timestamp(char* buf, int len)
{
	time_t now;

	time(&now);
	strftime(buf, len, "%h %e %T ", localtime(&now));

	return strlen(buf);
}
