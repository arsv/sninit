#include <time.h>

/* mktimestamp is roughly equivalent to strftime(buf, len, "%b %e %T", gmtime(ts)) */

/* A note on ts: gmtime(ts) == localtime(ts + tzoffset), and since sinit allows
   choosing either gmtime or localtime, this decision has been moved out of mktimestamp.
   
   RFC 5424 says a lot about the timestamp format, however, most implementations
   including busybox syslogd ahere to RFC 3164 and expect a *local* time
   formatted as "Jan 10 12:34:56", with no way to provide timezone offset.

   A notable exception is supposedly strict-standard-following musl, which
   unconditionally uses gmtime instead of localtime with the same RFC 3164
   string format. Thus using musl and non-musl executables in the same
   system will likely result in messed-up syslog timestamps.

   Using gmtime, regardless of its non-standard status, makes a lot of sense to me,
   so I'm leaving it as an option in sinit. */

static const char smonths[12][4];
static char* nto2s(char* p, int n, int z);

int mktimestamp(char* buf, int len, time_t ts)
{
	struct tm t;
	char* p = buf;
	/* gmtime from dietlibc is good, it's short and brings no dependencies */
	gmtime_r(&ts, &t);

	/* Jan 12 12:34:56  */
	/* 0123456789012345 */
	if(len < 16) return -1;

	strncpy(p, t.tm_mon < 12 ? smonths[t.tm_mon] : "???", 3); p += 3;

	*(p++) = ' '; p = nto2s(p, t.tm_mday, 1);
	*(p++) = ' '; p = nto2s(p, t.tm_hour, 0);
	*(p++) = ':'; p = nto2s(p, t.tm_min, 0);
	*(p++) = ':'; p = nto2s(p, t.tm_sec, 0);
	*(p++) = ' ';

	return (p - buf);
};

static const char smonths[12][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* 12 -> "12", 2 -> " 2" if z, "02" otherwise */
static char* nto2s(char* p, int n, int z)
{
	int h = (n / 10) % 10;
	*(p++) = (h || !z) ? '0' + h : ' ';
	*(p++) = '0' + (n%10);
	return p;
};
