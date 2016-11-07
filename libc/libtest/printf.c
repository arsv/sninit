/*
   This is printf implementation for tests.
   Only fd 1 output, and a minimalistic format support:
   	%Ni %0NX %s %p %m
   Multiple write()s are used despite the overhead, since overhead
   is completely irrelevant in this case.

   init itself relies exclusively on (v)snprintf, and these are (v)printf,
   so there is no name clash and both functions can safely reside in the
   same libc.a
*/

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/write.h>
#include <stdarg.h>
#include <errno.h>

static inline int skiptofmt(const char* s)
{
	const char* p;
	for(p = s; *p && *p != '%'; p++);
	return p - s;
}

static int itostr(char* buf, size_t len, long n, int base)
{
	short i;
	short w = 0;
	short s = 0;
	char digits[] = "0123456789ABCDEF";

	if(base == 10 && n < 0) { s = 1; n = -n; }

	len--;
	do {
		for(i = w < len ? w : len; i >= 1; i--)
			buf[i] = buf[i-1];
		buf[0] = digits[n % base];
		w++;  n /= base;
	} while(n); if(s) {
		for(i = w < len ? w : len; i >= 1; i--)
			buf[i] = buf[i-1];
		buf[0] = '-';
		w++;
	}
	len++;

	return w < len ? w : len;
}

#define F_ZERO	(1<<0)
#define F_MINUS	(1<<1)
#define F_LONG	(1<<2)
#define IBUF 50

int vprintf(const char* fmt, va_list ap)
{
	int ret = 0;

	char c;
	int num;
	int flags;
	char ibuf[IBUF];
	char* str;

	int len;
	char* out;
	int wrt;

	while(*fmt) {
		if(*fmt == '%') {
			len = 0;
			num = 0;
			flags = 0;
infmt:			c = *(++fmt);
			if(c >= '0' && c <= '9') {
				if(!num && c == '0')
					flags |= F_ZERO;
				num = num*10 + (c - '0');
				goto infmt;
			} else switch(c) {
				case '-':
					flags |= F_MINUS;
					goto infmt;
				case 'l':
					flags |= F_LONG;
					goto infmt;
				case 's':
					str = va_arg(ap, char*);
					out = str ? str : "(null)";
					len = str ? strlen(str) : 6;
					break;
				case 'c':
					ibuf[0] = '\0' + va_arg(ap, int);
					out = ibuf; len = 1;
					break;
				case 'i':
					len = itostr(out = ibuf, IBUF, (flags & F_LONG) ? va_arg(ap, long) : va_arg(ap, int), 10);
					break;
				case 'X':
					len = itostr(out = ibuf, IBUF, (flags & F_LONG) ? va_arg(ap, unsigned long) : va_arg(ap, unsigned), 16);
					break;
				case 'p':
					len = itostr(out = ibuf, IBUF, (size_t)va_arg(ap, void*), 16);
					break;
				case 'm':
					out = strerror(errno);
					len = strlen(out);
					break;
				default:
					break;
			}
			fmt++;
		} else {
			len = skiptofmt(out = (char*)fmt);
			fmt += len;
		} if(len) {
			if((wrt = syswrite(1, out, len)) < 0)
				return -1;
			else
				ret += wrt;
		}
	}

	return ret;
}

int printf(const char* fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vprintf(fmt, ap);
	va_end(ap);

	return ret;
}
