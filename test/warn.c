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

#define LOG_NOTICE	5
#define LOG_DAEMON	3<<3

int main(void)
{
	/* User mode test. The line should be sent, verbatim, to warnfd */
	wrptr = 0;
	warnfd = 3;
	T(warn("some line here"));
	A(warnfd == 3);
	A(wrptr == 1);
	S(wrlog[0].buf, "some line here");
	Eq(wrlog[0].fd, warnfd, "%i");

	/* User mode test, now with some formatting */
	wrptr = 0;
	warnfd = 3;
	T(warn("some line here: %i", 123));
	A(warnfd == 3);
	A(wrptr == 1);
	S(wrlog[0].buf, "some line here: 123");
	Eq(wrlog[0].fd, warnfd, "%i");

	/* Regular use, only syslog should be written to */
	wrptr = 0;
	warnfd = 2;
	T(warn("general message"));
	A(warnfd == 2);
	A(wrptr == 1);
	S(wrlog[0].buf, "<29>Jan 12 12:34:56 init: general message");
	Eq(wrlog[0].fd, syslogfd, "%i");

	/* Failing syslog; the message should be sent to warnfd too */
	wrptr = 0;
	warnfd = 2;
	flags = BREAKSYSLOG;
	T(warn("general message"));
	A(warnfd == 2);
	A(wrptr == 2);
	S(wrlog[0].buf, "<29>Jan 12 12:34:56 init: general message");
	Eq(wrlog[0].fd, syslogfd, "%i");
	S(wrlog[1].buf, "init: general message");
	Eq(wrlog[1].fd, warnfd, "%i");

	/* Failing warnfd; close and seal it */
	wrptr = 0;
	warnfd = 3;
	flags = BREAKWARNFD;
	T(!warn("general message"));
	A(warnfd == -1);
	A(wrptr == 1);
	S(wrlog[0].buf, "general message");
	Eq(wrlog[0].fd, 3, "%i");

	/* Very very long message */
	char* p = "---*---++---++---*---"; /* 21 */
	warnfd = 3;
	flags = 0;
	T(warn("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", p, p, p, p, p, p, p, p, p, p, p, p, p, p, p));

	/* Syslog-only message */
	wrptr = 0;
	warnfd = 3;
	T(warn("#general message"));
	A(wrptr == 1);
	S(wrlog[0].buf, "<29>Jan 12 12:34:56 init: general message");
	Eq(wrlog[0].fd, syslogfd, "%i");

	return 0;
}
