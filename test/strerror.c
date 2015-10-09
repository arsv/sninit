#include <errno.h>
#include "_test.h"

#define CHECK(e) {\
	char* r = strerror(e);\
	if(r == NULL) \
		printf("%s:%i: OK unknown error %s\n", __FILE__, __LINE__, #e);\
	else if(!strcmp(r, #e)) \
		printf("%s:%i: OK %s = \"%s\"\n", __FILE__, __LINE__, #e, r);\
	else \
		printf("%s:%i: FAIL expected %s got \"%s\"\n", __FILE__, __LINE__, #e, r);\
}

extern char* strerror(int err);

/* Current strerror implementation uses fixed-width char[] for error names,
   which means the compiler won't warn us if there's no place for \0 there. */

/* We do not care much about telinit or run errors, and those use switch()es
   anyway, so point in testing them. */

int main(void)
{
	CHECK(ENOENT);
	CHECK(ELOOP);
	CHECK(EADDRINUSE);
	CHECK(ENFILE);
	CHECK(ENOSPC);
	CHECK(EPIPE);
	CHECK(ENOMEM);
	CHECK(EINTR);
	CHECK(EINVAL);
	CHECK(EAGAIN);
	CHECK(EACCES);
	CHECK(ENOEXEC);
	CHECK(ENOTDIR);
	CHECK(E2BIG);
	CHECK(EIO);	
	CHECK(EPERM);

	CHECK(ENAMETOOLONG);	/* fitting constant name */
	//CHECK(EWOULDBLOCK);	/* it's EAGAIN */

	CHECK(2000);		/* non-existant value */

	return 0;
}
