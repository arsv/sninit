#define _ATFILE_SOURCE
#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "init.h"

/* exec() supplied initrect. Called from within a child */

extern struct config* cfg;		/* we need environment here */

void execinitrec(struct initrec* p)
{
	/* C_NULL: close fds 0, 1, 2 unless specified otherwise */
	int maxnullfd = 2;	

	setsid();

	if(p->flags & C_TTY)
		ioctl(0, TIOCSCTTY, 0);

	if(*p->name && p->flags & C_LOG) {
		int logfd;
		int dirfd = open(LOGDIR, O_DIRECTORY);

		if(dirfd >= 0) {
			logfd = openat(dirfd, p->name, O_WRONLY | O_CREAT | O_NOFOLLOW, 0600);
			if(logfd >= 0) {
				dup2(logfd, 1);
				dup2(logfd, 2);
				if(logfd > 2) close(logfd);
			} else {
				warn("%s[%i]: openat(%s) failed: %m\n", p->name, getpid(), p->name);
			}
			close(dirfd);
		} else {
			warn("%s[%i]: can't open %s: %m", p->name, getpid(), LOGDIR);
		}

		/* "log,null" should null stdin but leave stdout/stderr either unchanged or
		   redirected to /var/log/name */
		maxnullfd = 0;
	};

	if(p->flags & C_NULL) {
		int exfd, fd = open("/dev/null", O_RDWR);
		if(fd >= 0) {
			for(exfd = 0; exfd <= maxnullfd; exfd++)
				dup2(fd, exfd);
			if(fd > 2)
				close(fd);
		} else {
			warn("%s[%i]: can't open /dev/null: %i", p->name, getpid(), p->argv[0]);
		}
	}

	execve(p->argv[0], p->argv, cfg->env);
	warn("%s[%i] exec(%s) failed: %m", p->name, getpid(), p->argv[0]);
	_exit(-1);
};
