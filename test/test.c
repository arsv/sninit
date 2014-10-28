#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>

void die(const char* fmt, ...) __attribute__((noreturn));

int warn(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
	return 0;
}

void die(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
	kill(getpid(), SIGTERM);	/* debugger trap */
	_exit(-1);
}

/* Some tests need writable string literals, normally done with strdup.
   It looks like a minor nuisance, but turns out there's no easy way
   to do it without bringing malloc() family in.

   I can't find a way to ask gcc to put string literals to .data
   instead of .text, which is a shame since that would solve the problem
   completely and with grace.
   And strdupa is surprisingly messy, with gcc builtins and whatnot.

   So, here comes a simple kind-of malloc implementation, capable *just*
   enough to handle strdup calls.
   
   I don't feel like putting this into bundled libc. Why do, after all,
   if it works, it will works with native libc just as well, and opaque malloc
   pointers is not something I want in tests.
   Testing for use-after-free is out of question of course, since there's no free(). */

/* Max memory is set to be 8k, that's actually much more that it will use.
   Most of these calls are for small strings. */

#define HEAP 8*1024
int heaplen = HEAP;
int heapptr = 0;
char heap[HEAP];

static char* heapdupstr(const char* s)
{
	int l = strlen(s);
	int p = heapptr;

	if(p + l + 1 > heaplen)
		die("HEAP out of memory");
	memcpy(heap + p, s, l+1);
	heapptr += l + 1;	

	return heap + p;
}

char* heapdup(const char* s)
{
	if(!s) die("HEAP dup(NULL)");
	return heapdupstr(s);
}

char* heapdupnull(const char* s)
{
	return s ? heapdup(s) : NULL;
}
