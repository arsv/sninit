#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

/* These two must match init_warn.c */
#define MSGBUF 120
#define HDRBUF 50

#define BREAKSYSLOG 1
#define BREAKWARNFD 2

int warnfd;
int syslogfd = 123;
int flags = 0;

#define NLOG 20
#define NBUF 200

int wrptr;
struct wrlog {
	int fd;
	char buf[NBUF];
} wrlog[NLOG];

int writefullnl(int fd, char *buf, size_t count)
{
	int len = count < NBUF ? count : NBUF;
	wrlog[wrptr].fd = fd;
	memcpy(wrlog[wrptr].buf, buf, len);
	wrlog[wrptr].buf[len] = '\0';
	wrptr++;
	return (flags & BREAKWARNFD ? -1 : 0);
}

int writesyslog(const char* buf, int count)
{
	int len = count < NBUF ? count : NBUF;
	wrlog[wrptr].fd = syslogfd;
	memcpy(wrlog[wrptr].buf, buf, len);
	wrlog[wrptr].buf[len] = '\0';
	wrptr++;
	return (flags & BREAKSYSLOG ? -1 : 0);
}

int timestamp(char* buf, int len)
{
	char* stamp = "Jan 12 12:34:56 ";
	int slen = strlen(stamp);
	if(len < slen) slen = len;
	memcpy(buf, stamp, slen);
	return slen;
}

/* The following will only pass if

   	WARNPRIO = "<29>"
	WARNTAG = "init: "

   It's easy to change test strings below to use the constants,
   but that would make the tests less readable. Few people are going
   to change those values anyway, so why bother. */

int main(void)
{
	/* User mode test. The line should be sent, verbatim, to warnfd */
	wrptr = 0;
	warnfd = 3;
	warn("some line here");
	ASSERT(warnfd == 3);
	ASSERT(wrptr == 1);
	STREQUALS(wrlog[0].buf, "some line here");
	INTEQUALS(wrlog[0].fd, warnfd);

	/* User mode test, now with some formatting */
	wrptr = 0;
	warnfd = 3;
	warn("some line here: %i", 123);
	ASSERT(warnfd == 3);
	ASSERT(wrptr == 1);
	STREQUALS(wrlog[0].buf, "some line here: 123");
	INTEQUALS(wrlog[0].fd, warnfd);

	/* Regular use, only syslog should be written to */
	wrptr = 0;
	warnfd = 2;
	warn("general message");
	ASSERT(warnfd == 2);
	ASSERT(wrptr == 1);
	STREQUALS(wrlog[0].buf, "<29>Jan 12 12:34:56 init: general message");
	INTEQUALS(wrlog[0].fd, syslogfd);

	/* Failing syslog; the message should be sent to warnfd too */
	wrptr = 0;
	warnfd = 2;
	flags = BREAKSYSLOG;
	warn("general message");
	ASSERT(warnfd == 2);
	ASSERT(wrptr == 2);
	STREQUALS(wrlog[0].buf, "<29>Jan 12 12:34:56 init: general message");
	INTEQUALS(wrlog[0].fd, syslogfd);
	STREQUALS(wrlog[1].buf, "init: general message");
	INTEQUALS(wrlog[1].fd, warnfd);

	/* Failing warnfd; close and seal it */
	wrptr = 0;
	warnfd = 3;
	flags = BREAKWARNFD;
	warn("general message");
	ASSERT(warnfd == -1);
	ASSERT(wrptr == 1);
	STREQUALS(wrlog[0].buf, "general message");
	INTEQUALS(wrlog[0].fd, 3);

	/* Very very long message */
	char* p = "---*---++---++---*---"; /* 21 */
	warnfd = 3;
	flags = 0;
	warn("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", p, p, p, p, p, p, p, p, p, p, p, p, p, p, p);

	return 0;
}
