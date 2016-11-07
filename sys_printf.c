/* A simple vsnprintf implementation */

/* dietlibc complains (rightfully) about the full *printf code being bloated.
   Luckily, init does not need all that stuff. Only (v)snprintf is used,
   and only with %s %i %c %m placeholders; only limited width support is needed.
   So the following is a minimalistic implementation that provides just enough
   of dietlibc printf for init to use.

   Keeping printf-style format for warn() messages allows seamless substitution
   of a full-size printf if necessary, in particular when shared libc is used. */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>
#include "scope.h"

int errno = 0;

export int snprintf(char* buf, size_t len, const char* fmt, ...);
export int vsnprintf(char* buf, size_t len, const char* fmt, va_list ap);

local int skiptofmt(const char* s)
{
	const char* p;
	for(p = s; *p && *p != '%'; p++);
	return p - s;
}

local int strlncpy(char* dst, size_t dstspace, const char* src, size_t srclen)
{
	int cpn = srclen < dstspace ? srclen : dstspace;
	memcpy(dst, src, cpn);
	return cpn;
}

local int itostr(char* buf, size_t len, int n)
{
	short i;
	short w = 0;
	short s = n < 0 ? 1 : 0; n = n < 0 ? -n : n;

	len--;
	do {
		for(i = w < len ? w : len; i >= 1; i--)
			buf[i] = buf[i-1];
		buf[0] = '0' + (n % 10);
		w++;  n /= 10;
	} while(n); if(s) {
		for(i = w < len ? w : len; i >= 1; i--)
			buf[i] = buf[i-1];
		buf[0] = '-';
		w++;
	}
	len++;

	return w < len ? w : len;
}

int vsnprintf(char* buf, size_t len, const char* fmt, va_list ap)
{
	int stf;
	int cpn;
	int ret = 0;
	int pad;
	const char* arg;

	len--; /* for terminating \0 */
	while(*fmt && len) {
		if(*fmt == '%') {
			fmt++;
			pad = 0;
refmt:			switch(*(fmt++)) {
				case '-':
					/* %-*s for compatibility with common libc printfs */
					goto refmt;
				case '*':
					pad = va_arg(ap, int);
					goto refmt;
				case 's':
					arg = va_arg(ap, const char*);
					if(!arg) arg = "(null)";
					cpn = strlncpy(buf, len, arg, strlen(arg));
					break;
				case 'i':
					cpn = itostr(buf, len, va_arg(ap, int));
					break;
				case 'c':
					if(len > 0) *buf = (char)va_arg(ap, int);
					cpn = len > 0 ? 1 : 0;
					break;
				case 'm':
					if((arg = strerror(errno)))
						cpn = strlncpy(buf, len, arg, strlen(arg));
					else
						cpn = itostr(buf, len, errno);
					break;
				case '\0':
					/* the pointer has been moved beyond string boundary,
					   got to bring it back */
					fmt--;	
				default:
					/* can't risk guessing stack contents here, so bail out */
					*buf = '\0';
					return ret;
			} while(cpn < pad && cpn < len)
				buf[cpn++] = ' ';
		} else {
			stf = skiptofmt(fmt);
			cpn = strlncpy(buf, len, fmt, stf);
			fmt += cpn;
		}
		len -= cpn;
		ret += cpn;
		buf += cpn;
	}
	*buf = '\0';

	return ret;
}
