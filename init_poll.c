#define _GNU_SOURCE
#include <poll.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "init.h"
#include "scope.h"

/* FD polling and telinit communication.

Call order:
   main() calls pollfds()
   pollfds sets state |= S_INITCTL
   main checks for state & S_INITCTL and calls acceptctl()

The separation of pollfds and acceptctl is necessary to reap deceased
children before interpreting commands. That, in turn, is not really
necessary but makes the output clear, both for ? and for child control.

The actual command processing happens in init_cmds.c, these routines
only receive them. */

extern int state;
extern int initctlfd;
extern int timetowait;
extern sigset_t defsigset;
extern int warnfd;

export void pollfds(void);
export void acceptctl(void);

extern void parsecmd(char* cmd);
local int setsockopti(int fd, int opt, int val);
local void readcmd(int fd);


/* called from the main loop */
/* timetowait may be set by start() and spawn() */
void pollctl(void)
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

	if(r < 0 && errno != EINTR) {
		/* Failed ppoll means the main loop becomes unconstrained,
		   making init uncontrollable and wasting cpu cycles.
		   To avoid that, let's try to slow things down a bit. */
		warn("poll failed: %m");

		sigset_t cursigset;
		pts.tv_sec = timetowait >= 0 ? timetowait : 1;
		pts.tv_nsec = 0;

		/* ppoll also handles sigmask-lifting, try to work around that */
		sigprocmask(SIG_SETMASK, &defsigset, &cursigset);
		nanosleep(&pts, NULL);
		sigprocmask(SIG_SETMASK, &cursigset, NULL);

		/* EINTR, on the other hand, is totally ok (SIGCHLD etc) */
	} else if(r > 0) {
		/* only one fd in pfd, so not that much choice here */
		state |= S_INITCTL;
	}
}

/* We've got a pending connection on initctlfd, ppoll tells us.
   Accept it, and handle whatever command is there.

   Only one command is accepted for each connection. To send more, telinit
   must re-connect. This is to avoid implementing half-duplex communication
   here in init; instead, kernel-side connection state is used to manage
   data direction.

   Commands are always supplemented with credentials passed as ancilliary
   data. See unix(7) for explaination.

   The use of alarm (setitimer) is tricky here: it is pointless for the root
   user, however it is possible for a non-root user to make a connection and
   let it hang without sending anything, blocking init operation.
   To counteract that, telinit connection time is limited. */

void acceptctl(void)
{
	int fd;
	struct sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	struct itimerval it = {
		.it_interval = { 0, 0 },
		.it_value = { INITCTL_TIMEOUT, 0 }
	};
	
	/* initctlfd is SOCK_NONBLOCK */
	while((fd = accept(initctlfd, (struct sockaddr*)&addr, &addr_len)) > 0) {
		setitimer(ITIMER_REAL, &it, NULL);
		setsockopti(fd, SO_PASSCRED, 1);
		readcmd(fd);
		shutdown(fd, SHUT_WR);
		close(fd);
	}

	state &= ~S_INITCTL;
}

/* telinit must provide user's credentials in ancillary data,
   as init should only accept commands from root when running with S_PID1.
   See unix(7). */
void readcmd(int fd)
{
	bss char cbuf[CMDBUF];
	bss char mbuf[CMSG_SPACE(sizeof(struct ucred))];
	bss struct iovec iov[1] = { {
		.iov_base = cbuf,
		.iov_len = CMDBUF
	} };
	bss struct msghdr mhdr = {
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

int setsockopti(int fd, int opt, int val)
{
	return setsockopt(fd, SOL_SOCKET, opt, &val, sizeof(val));
}
