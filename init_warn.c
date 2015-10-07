#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <errno.h>
#include "config.h"
#include "init.h"
#include "scope.h"

#define MSGBUF 120
#define HDRBUF 50

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
   snprintf stuff in a single location. */

int warnfd = 0;

export int warn(const char* fmt, ...);

/* warn() essentially includes a simple syslog() implementation.
   Full syslog() is not needed here, and neither are redundant sigprocmask()
   calls. Also, using library syslog(3) with sys_printf.c is a bad idea. */

local int syslogfd = -1;	/* not yet opened */
local int syslogtype;
local struct sockaddr syslogaddr = {
	.sa_family = AF_UNIX,
	.sa_data = SYSLOG
};

local int writefullnl(int fd, char *buf, size_t count);
local int writesyslog(const char* buf, int count);
extern int timestamp(char* buf, int len);

/* it's in RFC 3164 and we're not going to include syslog.h
   just because of two puny constants */
#define LOG_NOTICE	5
#define LOG_DAEMON	3<<3

local int warnmode(const char* fmt);

#define W_WARNFD	1<<0
#define W_SYSLOG	1<<1
#define W_STDERR	1<<2
#define W_SKIP		1<<3

/* Stderr output needs "init:" prefixed to the message,
   and syslog needs its own prefix in addition to that.

        |--------hdr--------|-tag-|-----------msg------------||
        <29>Jan 10 12:34:56 init: crond[123] abnormal exit 67↵₀

   The final message is formed with all prefixes needed for a given
   mode mask (W_SYSLOG and/or W_STDERR).
   If it comes down to a less demanding modes, extra prefixes are skipped
   by passing (buf + prefixlen) to respective write* function. */

int warn(const char* fmt, ...)
{
	va_list ap;
	bss char buf[HDRBUF+MSGBUF+2];
	int hdrlen;
	int msglen;
	int taglen;

	int origerrno = errno;	/* timestamp() may overwrite it */
	short mode = warnmode(fmt);

	if(!mode) return -1;
	if(mode & W_SKIP) fmt++;

	if(mode & W_SYSLOG) {
		hdrlen = snprintf(buf, HDRBUF, "<%i>", LOG_DAEMON | LOG_NOTICE);
		hdrlen += timestamp(buf + hdrlen, HDRBUF - hdrlen);
	} else {
		hdrlen = 0;
	}

	taglen = snprintf(buf + hdrlen, HDRBUF - hdrlen, "init: ");

	errno = origerrno;
	va_start(ap, fmt);
	msglen = vsnprintf(buf + hdrlen + taglen, MSGBUF, fmt, ap);
	va_end(ap);

	/* buf has one more byte at the end, for line end in case of console/socket output,
	   and possibly for record terminator in case of syslog;
	   writefullnl and writesyslog take care of it */

	if(mode & W_WARNFD)
		if(writefullnl(warnfd, buf + hdrlen + taglen, msglen))
			return (warnfd = -1); /* socket connection lost */

	if(mode & W_SYSLOG)
		if(!writesyslog(buf, hdrlen + taglen + msglen))
			return 0;

	if(mode & W_STDERR)
		return writefullnl(2, buf + hdrlen, taglen + msglen);
	
	return 0;
}

/* During telinit request, warnfd is the open telinit connection.
   In case connection fails, we set warnfd to -1 to "lock" warn,
   preventing further attempts to write anything until current telinit
   session is over.

   Without active telinit connection init output should be sent to syslog
   if possible. Generally we should try to contact syslog to check that,
   but during early boot and late shutdown it is clear syslogd is not running,
   so we skip that by setting warnfd = 0.

   Finally, ! at the start of fmt indicates the message should go to stderr,
   skipping syslog. This is for debugging only. There is also # prefix which
   is *not* handled here and must be passed to telinit. */

int warnmode(const char* fmt)
{
	if(*fmt == '!')
		return W_STDERR | W_SKIP;
	if(warnfd > 2)
		return W_WARNFD;
	if(warnfd == 2)
		return W_SYSLOG | W_STDERR;
	if(warnfd < 0)
		return 0;		/* warn locked */
	else
		return W_STDERR;	/* do not even try syslog */
}

/* Syslog socket may be either STREAM or DGRAM, and warnfd may happen to be
   a stream socket as well. With sockets, incomplete write()s are possible
   and must be handled. */

int writefullnl(int fd, char *buf, size_t count)
{
	int r = 0;

	*(buf + count++) = '\n'; /* terminate the line, see comments in warn() */
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

int tryconnectsyslog(int type)
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

int writesyslog(const char* buf, int count)
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
