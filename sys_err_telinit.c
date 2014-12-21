#include <stdlib.h>
#include <errno.h>

/* Error messages for telinit. See sys_err_init.c for explaination. */

/* Note telinit does not use sys_printf, and expects strerror to do
   message-or-number trick itself. NULL should never be returned. */

#define r(sym) case sym: return #sym

char* strerror(int err)
{
	static char buf[10];

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
		r(EPERM);	/* sendmsg, bad build */
		r(EIO);		/* unlikely, bad news */
	}

	int i = sizeof(buf)-1;
	buf[i--] = '\0';
	while(err && i >= 0) {
		buf[i--] = '0' + (err % 10);
		err /= 10;
	};

	return buf + i + 1;
}
