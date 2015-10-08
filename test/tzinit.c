#include <time.h>
#include "_test.h"

extern const char* tzfile;

extern void tzinit(time_t base);
extern void tzparse(unsigned char* buf, int len, time_t t0);

extern struct {
	int ts;
	int te;
	int dt;
	char set;
} tzinfo;

time_t tztime;

int main(void)
{
	memset(&tzinfo, 0, sizeof(tzinfo));

	/* Regular entry */
	tzfile = "tzinit_1.bin";
	tztime = 1407707612;
	tzinit(tztime);
	A(tzinfo.set);
	A(tzinfo.ts == 1396141200);
       	A(tzinfo.te == 1414285200);
	A(tzinfo.dt == 10800);

	/* Final entry in that file, let's see how it handles unbounded interval */
	/* Relevant interval is last-transition to +inf */
	tztime = 2140045300;
	tzinfo.set = 0;
	tzinit(tztime);
	A(tzinfo.set);
	A(tzinfo.ts == 2140045200);
	A(tzinfo.te == 0);
	A(tzinfo.dt == 7200);

	/* First entry in the file, got to select types[0] */
	/* Relevant inverval is -inf to the first transition time */
	tztime = -1441159324; /* (May 1924, really?) */
	tzinfo.set = 0;
	tzinit(tztime);
	A(tzinfo.set);
	A(tzinfo.ts == 0);
	A(tzinfo.te == -1441159324);
	A(tzinfo.dt == 7324);

	/* Badly truncated TZ file */
	tzinfo.ts = 1234;
	tzinfo.te = 5678;
	tzinfo.dt = 1122;
	tzfile = "tzinit_0.bin";
	tztime = 1407707612;
	tzinit(tztime);
	A(tzinfo.set);
	A(tzinfo.ts == 1234);	/* the correct behavior is not to change ts/te/dt in case tzparse fails */
       	A(tzinfo.te == 5678);	/* (ok more like implemented behavior, not necessary correct) */
	A(tzinfo.dt == 1122);

	/* c out of bounds */
	tzinfo.ts = 1234;
	tzinfo.te = 5678;
	tzinfo.dt = 1122;
	tzfile = "tzinit_2.bin";
	tztime = 1407707612;
	tzinit(tztime);
	A(tzinfo.set);
	A(tzinfo.ts == 1234);	/* again, should keep the values */
       	A(tzinfo.te == 5678);
	A(tzinfo.dt == 1122);

	return 0;
}
