#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include "config.h"
#include "init.h"

/* Most of the output generated withint init is conditional with regard
   to where it actually goes.

       * Process events (died/killed/failed) should go to syslog,
         unless we can't contact syslogd, in which case they should
	 be sent to stderr.

       * Initial configuration errors should go to stderr, but any
         errors encountered during user-requested reconfiguration
	 should be sent back to telinit socket.

       * Responses to telinit requests should be sent back to telinit,
         unless it was a configutaion request which is already conditional.

   There is a lot of overlap between the cases, so the code calls warn()
   and everything else is decided within warn(). This also keeps nasty
   vsnprintf stuff in a single location. */

int warnfd = 0;

#define MSGBUF 120
#define HDRBUF 12

static char warnbuf[HDRBUF+MSGBUF+2];

/* warn() essentially includes a simple syslog() implementation.
   Full syslog() is not needed here, and neither are redundant sigprocmask()
   calls. Also, using library syslog(3) with sys_printf.c is a bad idea. */

static int syslogfd = -1;	/* not yet opened */
static int syslogtype;
static struct sockaddr syslogaddr = {
	.sa_family = AF_UNIX,
	.sa_data = SYSLOG
};

/* Syslog socket may be either STREAM or DGRAM, and warnfd may happen to be
   a stream socket as well. With sockets, incomplete write()s are possible
   and must be handled. */

static int writefullnl(int fd, char *buf, size_t count)
{
	int r = 0;

	*(buf + count++) = '\n'; /* terminate the line */
	while(count > 0) {
		r = write(fd, buf + r, count - r);
		if(r < 0)
			return r;
		count -= r;
	}

	return 0;
}

/* Syslog connection is reused whenever possible, otherwise attempts
   to open it are repeated on each warn() call. This way init can handle
   dying/respawning syslog gracefully and spare some extra syscalls. */

static int tryconnectsyslog(int type)
{
	if((syslogfd = socket(AF_UNIX, type, 0)) < 0)
		return -1;
	if(connect(syslogfd, &syslogaddr, sizeof(syslogaddr))) {
		close(syslogfd);
		syslogfd = -1;
		return -1;
	} else {
		syslogtype = type;
		return 0;
	}
}

static int writesyslog(const char* buf, int count)
{
	if(syslogfd >= 0)
		goto send;
	if(!tryconnectsyslog(SOCK_DGRAM))
		goto send;
	if(!tryconnectsyslog(SOCK_STREAM))
		goto send;
	return -1;
send:
	if(syslogtype == SOCK_STREAM)
		count++;	/* include terminating \0 */

	return (write(syslogfd, buf, count) < 0);
}

/* During telinit request, warnfd is the open telinit connection.
   In case connection fails, we set warnfd to -1 to "lock" warn,
   preventing further attempts to write anything until current telinit
   session is over.

   Without active telinit connection init output should be sent to syslog
   if possible. Generally we should try to contact syslog to check that,
   but during early boot and late shutdown it is clear syslogd is not running,
   so we skip that by setting warnfd = 0.

   Stderr output needs "init:" prefixed to the message, and syslog needs
   its own prefix in addition to that.

        |hdr-|-tag-|-----------msg------------||
        <29> init: crond[123] abnormal exit 67↵₀

   Extra prefixes are skipped by passing (buf + ...) to respective
   write* function.

   Priority code 29 means (LOG_DAEMON | LOG_NOTICE), see RFC 3164.
   Init never logs anything that is not NOTICE.

   Note current implementation does NOT include timestamp in the hdr part.
   See doc/syslog.txt on this. */

void warn(const char* fmt, ...)
{
	va_list ap;
	char* buf = warnbuf;

	if(warnfd < 0) return;

	const char* pri = "<29> ";    /* prefix for syslog only */
	int prilen = strlen(pri);
	strncpy(buf, pri, HDRBUF);

	const char* tag = "init: ";  /* prefix for syslog and stderr*/
	int taglen = strlen(tag);
	strncpy(buf + prilen, tag, HDRBUF - prilen);

	va_start(ap, fmt);
	int msglen = vsnprintf(buf + prilen + taglen, MSGBUF, fmt, ap);
	va_end(ap);

	if(warnfd > 2) {
		/* telinit connection */
		if(writefullnl(warnfd, buf + prilen + taglen, msglen))
			warnfd = -1; /* socket connection lost */
		return;
	}

	if(warnfd == 2)
		/* try syslog, fall back to stderr */
		if(!writesyslog(buf, prilen + taglen + msglen))
			return;

	writefullnl(2, buf + prilen, taglen + msglen);
}
