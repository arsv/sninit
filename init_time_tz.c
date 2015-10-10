/* Handling time and timezone data, for syslog output */

/* dietlibc implementation of localtime keeps the whole timezone file
   mmaped, with MAP_PRIVATE for no apparent reason.
   
   This would make sense if formatting arbitrary time is to be considered,
   but init does not need to format arbitrary timestamps.
   All timestamps in init are expected to be small (well about uptime small)
   increments from the boot time. 

   So the idea is to mmap timezone, calculate current offset as well as its
   validity boundaries, unmap timezone, and use this offset for as long as
   time stays within the boundaries.
   Once the time goes out of bondaries, timezone is mmaped again and so on. */

#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "init.h"
#include "config.h"
#include "scope.h"

export int timestamp(char* buf, int len);
extern int mktimestamp(char* p, int l, time_t ts);

/* To redefine it for testing */
local const char* tzfile = LOCALTIME;

int tzlock = 0;

local struct {
	int ts;
	int te;
	int dt;
} tzinfo; /* assumed to be zero-initialized */

local void tzinit(time_t base);
local void tzparse(unsigned char* buf, int len, time_t t0);

/* This is somewhat illogical to do heavy file operations
   within a call that is supposed to make a simple timestamp.
   However, handling tzinit() properly involves a lot of changes
   in unrelated parts of init, while the result barely affects
   anything. And after all, dietlibc does more or less the same. */

/* WARNING: timestamp is called from warn()! */

int timestamp(char* buf, int len)
{
	struct timespec tp = { 0, 0 };
	
	clock_gettime(CLOCK_REALTIME, &tp);

	if(!tzlock
	|| (tzinfo.ts && tp.tv_sec < tzinfo.ts)
	|| (tzinfo.te && tp.tv_sec < tzinfo.te))
		tzinit(tp.tv_sec);

	return mktimestamp(buf, len, tp.tv_sec + tzinfo.dt);
}

/* Load /etc/localtime, initializing tzinfo structure above */
/* Should have been (strong) void tzset(), but alas, tzset happens
   to be a strong symbol in dietlibc. */
void tzinit(time_t base)
{
	int fd;
	struct stat st;
	unsigned char* buf;

	/* Do not try to re-load timezone after a failure. */
	tzlock = 1;

	/* Errors here are not reported, we just bail out */

	if((fd = open(tzfile, O_RDONLY)) < 0)
		return;
	if(fstat(fd, &st))
		goto out;
	if((buf = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
		goto out;

	tzparse(buf, st.st_size, base);

	munmap(buf, st.st_size);
out:	close(fd);
	return;
}

/* big endian 4-byte int at */
int beint32(unsigned char* p)
{
	return (p[0] << 3*8) | (p[1] << 2*8) | (p[2] << 1*8) | p[3];
};

#if 0
/* See tzinfo(5) for a verbose description. Sadly, C does not allow
   variable structures, so this is only for reference. */

struct tzfile
{
	char magic[4];		/* "TZif" */
	char version[1];	/* '\0' or '2' */
	char _[15];

	long tzh_ttisgmtcnt;
	long tzh_ttisstdcnt;
	long tzh_leapcnt;
	long tzh_timecnt;
	long tzh_typecnt;
	long tzh_charcnt;

	long tzh_times[tzh_timecnt];
	unsigned char tzh_ttype[tzh_timecnt];
	
	struct ttinfo {
		long tt_gmtoff;
		unsigned char tt_isdst;
		unsigned char tt_abbrind;
	} tzh_types[tzh_typecnt];

	struct tzleap {
		long base;
		long leap;
	} tzh_leaps[tzh_leapcnt];

	unsigned char tzh_ttisstd[tzh_ttisstdcnt];
	unsigned char tzh_ttisgmt[tzh_ttisgmtcnt];
}
#endif

/* tzfile is mmaped /etc/localtime; t is current/reference time */
/* Warning: called from warn()! No error reporting here. */
void tzparse(unsigned char* buf, int len, time_t t)
{
	int i;

	if(len < 50 || memcmp((char*)buf, "TZif", 4))
		return;

	int tzh_timecnt = beint32(buf + 20 + 3*4);
	int tzh_typecnt = beint32(buf + 20 + 4*4);

	/* No data (?!) */
	if(!tzh_timecnt)
		return;
	/* Bogus length? */
	if(tzh_timecnt < 0 || tzh_typecnt < 0)
		return;
	/* expected length, given *count values so far */
	int expfilelen = 20 + 6*4 		/* header and counters */
		+ tzh_timecnt*4			/* tzh_times */
		+ tzh_timecnt*1			/* tzh_ttype */
		+ (4 + 1 + 1)*tzh_typecnt;	/* tzh_types */
	if(expfilelen > len)
		return;

	unsigned char* tzh_times = buf + 20 + 6*4;
	for(i = 0; i < tzh_timecnt; i++)
		if((time_t)beint32(tzh_times + i*4) >= t)
			break;
	/* And btw, NOT breaking anywhere is ok for us as well, it means
	   there's no upper boundary for current time. */

	int dt = 0;
	/* note i is the *next* interval index */
	unsigned char* tzh_ttype = tzh_times + 4*tzh_timecnt;
	unsigned char c = i > 0 ? tzh_ttype[i - 1] : 0;

	if(c > tzh_typecnt)
		return;

	unsigned char* tzh_types = tzh_ttype + tzh_timecnt;
	dt = beint32(tzh_types + c*6);

	tzinfo.dt = dt;
	tzinfo.ts = i > 0 ? (time_t)beint32(tzh_times + (i-1)*4) : 0;
	tzinfo.te = i < tzh_timecnt ? (time_t)beint32(tzh_times + i*4) : 0;
}
