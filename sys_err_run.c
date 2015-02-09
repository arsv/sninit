#include <stdlib.h>
#include <errno.h>

/* Error messages for run. See sys_err_init.c for explanation. */
/* This is similar to telinit */

extern char* ltoa(long n);

#define r(sym) case sym: return #sym

char* strerror(int err)
{
	switch(err) {
		r(EAGAIN);	/* setresuid */
		r(EPERM);	/* setresuid */
		r(EFAULT);	/* open */
		r(EINVAL);	/* any, bad news */
		r(ELOOP);	/* open, unlikely */
		r(EACCES);	/* open */
		r(EISDIR);	/* open */
		r(EMFILE);	/* open */
		r(ENFILE);	/* open */
		r(ENOENT);	/* open */
		r(ENOMEM);	/* open */
		r(ENAMETOOLONG);/* open, unlikely */
		r(ENODEV);	/* mmap */
		r(EOVERFLOW);	/* mmap */
		r(EINTR);	/* write? */
		r(EIO);		/* fstat, write */
		r(EBADF);	/* fstat, mmap */
		default: return ltoa(err);
	}

}
