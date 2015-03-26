#include <stdlib.h>
#include <time.h>

int sleep(int sec)
{
	struct timespec ts = { sec, 0 };

	return nanosleep(&ts, NULL);
}
