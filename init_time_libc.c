#include <string.h>
#include <time.h>

/* This is fallback implementation using libc calls;
   see sys_timestamp.c, sys_time_tz.c, sys_time_notz.c */

int timestamp(char* buf, int len)
{
	struct timespec tp = { 0, 0 };

	clock_gettime(CLOCK_REALTIME, &tp);
	strftime(buf, len, "%h %e %T ", localtime(&tp.tv_sec));

	return strlen(buf);
}
