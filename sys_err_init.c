#include <stdlib.h>
#include <errno.h>

/* Reporting integer errno values is not nice, even in init, but linking libc sterror()
   brings in the whole syserrlist. This is while init itself can only
   produce a handful of different errno values.

   The idea is to choose the middle ground, providing readable messages for expected
   errors and printing anything unexpected as a integer. Getting something unexpected
   in init is already a sign of a big trouble, so making the user go on and use
   strerror(3)/perror(1)/etc to get the message should not make things much worse.

   Finally, I'm not a big fan of verbose messages like "No such file or directory",
   especially when they aren't accompanied by respective error code.
   Most man pages only list symbolic constants, ENOENT in this case, with
   a syscall-specific description, so searching for "No such file ..." is pointless.
   Showing symbolic constants, on the other hand, makes it easy to read both errno(3)
   and relevant section 2 pages
   (when it's clear from the message which syscall failed, that is) */

/* The whole symbol-or-int thing only works via sys_printf %m.
   dietlibc printf can't handle NULL return properly, and neither glibc
   nor musl printfs use strerror internally. */

/* For a long time this has been a switch(err), but it turns out gcc generates
   relatively large jump table here. The same does not seem to happen with telinit
   and run errlists, strange. */

struct errlist {
	unsigned char err;
	char name[11];	/* EADDRINUSE is 10+1 bytes long */
} errlist[] = {
#define r(e) { e, #e }
	r(ENOENT),	/* open */
	r(ELOOP),	/* open, unlikely */
	r(EADDRINUSE),	/* bind */
	r(ENFILE),	/* open, socket, accept */
	r(ENOSPC),	/* open(..., O_CREAT) for process logging */
	r(EPIPE),	/* sendmsg */
	r(ENOMEM),	/* mmap and a buch of other calls */
	r(EINTR),	/* poll, unlikely for other syscalls */
	r(EINVAL),	/* generic, unlikely */
	r(EAGAIN),	/* fork */
	r(EACCES),	/* execve */
	r(ENOEXEC),	/* execve */
	r(ENOTDIR),	/* execve */
	r(E2BIG),	/* execve, very large service file */
	r(EIO),		/* unlikely, bad news */
	r(EPERM)	/* reboot and some other calls */
};

#define endof(a) (a + sizeof(a)/sizeof(*a))

char* strerror(int err)
{
	struct errlist* p;

	for(p = errlist; p < endof(errlist); p++)
		if((int)p->err == err)
			return p->name;

	return NULL;
}
