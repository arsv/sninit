#include <time.h>

/* seconds per day */
#define SPD (24*60*60)

/* days per month -- nonleap! */
static const short spm[13] = {
	0,
	(31),
	(31+28),
	(31+28+31),
	(31+28+31+30),
	(31+28+31+30+31),
	(31+28+31+30+31+30),
	(31+28+31+30+31+30+31),
	(31+28+31+30+31+30+31+31),
	(31+28+31+30+31+30+31+31+30),
	(31+28+31+30+31+30+31+31+30+31),
	(31+28+31+30+31+30+31+31+30+31+30),
	(31+28+31+30+31+30+31+31+30+31+30+31)
};

static int isleap(int year)
{
	/* every fourth year is a leap year except for century years that are
	 * not divisible by 400. */
	return (!(year%4) && ((year%100) || !(year%400)));
}

struct tm* gmtime_r(const time_t* timep, struct tm* r)
{
	time_t i;
	time_t dd = *timep / SPD;	/* date */
	time_t tt = *timep % SPD;	/* time */

	/* Time */
	r->tm_sec = tt % 60; tt /= 60;
	r->tm_min = tt % 60; tt /= 60;
	r->tm_hour = tt;

	/* Weekday, ref. 1970 and not the current year */
	r->tm_wday = (4 + dd) % 7;

	/* Year */
	time_t dy;
	for(i = 1970; ; i++)
		if(dd >= (dy = isleap(i) ? 366 : 365))
			dd -= dy;
		else
			break;

	r->tm_year = i - 1900;
	r->tm_yday = dd;

	/* Month and day */
	r->tm_mday = 1;
	if(isleap(i) && (dd > 58)) {
		if(dd == 59)		/* Feb 29 */
			r->tm_mday = 2;
		dd -= 1;
	}

	for(i = 11; i && (spm[i] > dd); --i)
		;

	r->tm_mon = i;
	r->tm_mday += dd - spm[i];

	return r;
}
