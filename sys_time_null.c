/* The simpliest way to privide a timestamp in syslog messages
   is not to provide timestamp at all. RFC 3164 section 4.3.2
   mandates syslogd to insert a timestamp into the message,
   thus moving localtime/gmtime issues out of init.

   This particular part of RFC is also widely implemented,
   and not only for "relying" as RFC puts it.

   The primary drawback is that the messages will be tagged with
   syslog time, which may differ from init time. This may
   or may not be an issue though, especially considering syslogd
   is often a local process using the same system time. */

#include <time.h>

int timestamp(char* buf, int len)
{
	*buf = ' ';
	return 1;
}
