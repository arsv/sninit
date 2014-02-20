#define _GNU_SOURCE
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "init.h"

/* FD polling and telinit communication.

Call order:
   main() calls pollfds()
   pollfds sets state |= S_INITCTL
   main checks for state & S_INITCTL and calls acceptctl()

The separation of pollfds and acceptctl is necessary to reap deceased
children before interpreting commands.

Note that the actual command processing happens in init_cmds.c */

extern int state;
extern int initctlfd;
extern int timetowait;
extern sigset_t defsigset;
extern int warnfd;

extern void parsecmd(char* cmd);
int setsockopt_int(int fd, int opt, int val);
void readcmd(int fd);

/* called from the main loop */
/* timetowait may be set by start() and spawn() */
void pollfds(void)
{
	int r;
	struct pollfd pfd;	
	struct timespec pts;
	struct timespec* ppts;

	pfd.fd = initctlfd;
	pfd.events = POLLIN;
	if(timetowait >= 0) {
		pts.tv_sec = timetowait;
		pts.tv_nsec = 0;
		ppts = &pts;
	} else {
		ppts = NULL;
	}

	/* init spends most of its time here in ppoll(), waiting
	   either for signals or telinit requests. */
	/* See comments in setup() regarding defsigset. */
	r = ppoll(&pfd, 1, ppts, &defsigset);

	if(r < 0) {
		if(errno != EINTR)
			warn("poll failed: %m");
		/* EINTR, on the other hand, is ok (SIGCHLD etc) */
		return;
	} else if(r > 0) {
		/* only one fd in pfd, so not that much choice here */
		state |= S_INITCTL;
	}
}

/* We've got a pending connection on initctlfd.
   Accept it, and handle whatever command is there.

   Only one command is accepted for each connection. To send more, telinit
   must re-connect. This is to avoid implementing half-duplex communication
   here in init; instead, kernel-side connection state is used to manage
   data direction.

   Commands are always supplemented with credentials passed as ancilliary
   data. See unix(7) for explaination. */

void acceptctl(void)
{
	int fd;
	struct sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	
	/* initctlfd is SOCK_NONBLOCK */
	while((fd = accept(initctlfd, (struct sockaddr*)&addr, &addr_len)) > 0) {
		setsockopt_int(fd, SO_PASSCRED, 1);
		readcmd(fd);
		shutdown(fd, SHUT_WR);
		close(fd);
	}

	state &= ~S_INITCTL;
}

void readcmd(int fd)
{
	char cbuf[CMDBUF];
	char mbuf[CMSG_SPACE(sizeof(struct ucred))];
	struct iovec iov[1] = { {
		.iov_base = cbuf,
		.iov_len = CMDBUF + 1
	} };
	struct msghdr mhdr = {
		.msg_name = NULL,
		.msg_namelen = 0,
		.msg_iov = iov,
		.msg_iovlen = 1,
		.msg_control = mbuf,
		.msg_controllen = sizeof(mbuf),
		.msg_flags = 0
	};
	struct cmsghdr *cmsg;
	struct ucred *cred;
	ssize_t rb;

	rb = recvmsg(fd, &mhdr, 0);
	if(rb < 0)
		retwarn_("recvmsg failed: %m");

	cmsg = CMSG_FIRSTHDR(&mhdr);
	if(!cmsg)
		retwarn_("no ancilliary data (?)");

	if(!(cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_CREDENTIALS))
		retwarn_("ancilliary data isn't SCM_CREDENTIALS");

	if(cmsg->cmsg_len < CMSG_LEN(sizeof(*cred)))
		retwarn_("ancilliary data is too short");

	cred = (struct ucred*) CMSG_DATA(cmsg);
	if(cred->uid && (state & S_PID1))
		retwarn_("non-root access");

	if(rb <= 0)
		return;

	cbuf[rb] = '\0';

	warnfd = fd;
	parsecmd(cbuf);
	warnfd = 2;
}

int setsockopt_int(int fd, int opt, int val)
{
	return setsockopt(fd, SOL_SOCKET, opt, &val, sizeof(val));
}
