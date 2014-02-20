#include <stdlib.h>
#include <errno.h>

/* Error messages for telinit. See sys_err_init.c for explaination. */

#define r(sym) case sym: return #sym

char* strerror(int err)
{
	switch(err) {
		r(ENOENT);	/* open */
		r(ELOOP);	/* open, unlikely */
		r(ENFILE);	/* open, socket, accept */
		r(EPIPE);	/* sendmsg */
		r(ENOMEM);	/* mmap and a buch of other calls */
		r(ECONNREFUSED);/* connect in telinit */
		r(EINTR);	/* poll; unlikely for other syscalls */
		r(EINVAL);	/* generic, unlikely */
		r(EACCES);	/* execve */
		r(EIO);		/* unlikely, bad news */
		default: return NULL;
	}
}
