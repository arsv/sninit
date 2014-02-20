#include <stdio.h>
#include "test.h"

extern void tzinit(void);
extern void tzparse(unsigned char* buf, int len, time_t t0);

extern struct {
	int ts;
	int te;
	int dt;
} tzinfo;

/* Ok this is quite hard to actually *test* this without
   making /etc/localtime redefineable and making some kind of test tz files.
   However, given how irrelevant this actually is, let's just check
   the offsets are there. */

int main(void)
{
	tzinit();
	printf("ts=%i te=%i dt=%i\n", tzinfo.ts, tzinfo.te, tzinfo.dt);
	return 0;
}
