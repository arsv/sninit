#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include "config.h"
#include "init.h"

#define MSGBUF 120
#define HDRBUF 50

/* Generally init messages should be forwarded to syslog.
   However, in some situations it's better to write them to
   (or to copy them to) stderr, and sometimes also
   to warnfd when it's a connection to active telinit.
   
   In sinit, the value of warnfd is usually enough to decide where
   the message should go. For instance, syslog-only process status
   messages are never generated during reconfiguration.
   See the first if() block in warn() below.

   In case direct control is needed, desired logging mode is passed
   as a prefix to the format string: "#message for syslog only".

   wall-style messages ("system is going down" etc) are not supported.

   sinit tries to reopen syslogfd every time warn() is called,
   so not-yet-ready and/or failing syslogd shoudn't be a problem. */

extern int warnfd;
extern int syslogfd;
int syslogtype;
struct sockaddr syslogaddr = {
	.sa_family = AF_UNIX,
	.sa_data = SYSLOG
};

int writefullnl(int fd, char *buf, size_t count);
int writesyslog(const char* buf, int count);

extern int timestamp(char* buf, int len);

/* Note: warn() essentially includes a simple syslog() implementation.
   Full syslog() is not needed here, and neither are redundant sigprocmask() calls.
   Also, using library syslog(3) with sys_printf.c is a bad idea. */

#define W_WARNFD	1<<0
#define W_SYSLOG	1<<1
#define W_STDERR	1<<2

/* it's in RFC 3164 and we're not going to include syslog.h
   just because of two puny constants */
#define LOG_NOTICE	5
#define LOG_DAEMON	3<<3

int warn(const char* fmt, ...)
{
	va_list ap;
	char buf[HDRBUF+MSGBUF+2];
	int hdrlen;
	int msglen;
	int taglen;
	char over = ' ';
	short mode;

	/* Ok, got to decide where warn() should put the message */
	switch(*fmt) {
		case '#':
		case ' ':
		case '!':
			over = *(fmt++);
	} if(over == '#')
		mode = W_SYSLOG;		/* syslog only */
	else if(over == '!')
		mode = W_WARNFD | W_SYSLOG;	/* warnfd AND syslog; see W_* logic below */
	else if(warnfd > 2)
		mode = W_WARNFD;		/* user-action error */
	else if(warnfd == 2)
		mode = W_SYSLOG | W_STDERR;	/* try syslog of fall back to stderr */
	else if(warnfd < 0)
		return -1;			/* warn locked */
	else 
		mode = W_STDERR;		/* do not even try syslog */

	if(mode & W_SYSLOG) {
		hdrlen = snprintf(buf, HDRBUF, "<%i>", LOG_DAEMON | LOG_NOTICE);
		hdrlen += timestamp(buf + hdrlen, HDRBUF - hdrlen);
	} else {
		hdrlen = 0;
	}
	taglen = snprintf(buf + hdrlen, HDRBUF - hdrlen, "init: ");

	va_start(ap, fmt);
	msglen = vsnprintf(buf + hdrlen + taglen, MSGBUF, fmt, ap);
	va_end(ap);

	/* buf has one more byte at the end, for line end in case of console/socket output,
	   and possibly for record terminator in case of syslog;
	   writefullnl and writesyslog take care of it */

	if(mode & W_WARNFD)
		if(writefullnl(warnfd, buf + hdrlen + taglen, msglen))
			/* socket connection lost, lock warn() until further notice */
			return (warnfd = -1);

	if(mode & W_SYSLOG)
		/* process-status message */
		/* try to log if syslog is available, otherwise dump everything to stderr */
		if(!writesyslog(buf, hdrlen + taglen + msglen))
			return 0;

	/* either syslog write was unsuccessful, or it's too early/late to expect syslog */	
	if(mode & W_STDERR)
		return writefullnl(2, buf + hdrlen, taglen + msglen);
	
	return 0;
}

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

static int tryconnectsyslog(type)
{
	syslogtype = type;
	syslogfd = socket(AF_UNIX, type, 0);
	if(syslogfd < 0)
		return -1;
	return connect(syslogfd, &syslogaddr, sizeof(syslogaddr));
}

int writesyslog(const char* buf, int count)
{
	if(syslogfd < 0) {
		if(!tryconnectsyslog(SOCK_DGRAM))
			goto send;
		if(!tryconnectsyslog(SOCK_STREAM))
			goto send;
		return -1;
	}

send:
	if(syslogtype == SOCK_STREAM)
		count++;	/* include terminating \0 */

	return (send(syslogfd, buf, count, 0) < 0);
}
