#define _GNU_SOURCE
#include <poll.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "config.h"
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
local int checkuser(int fd);
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

   Alarm (setitimer) is needed here to force-reset a hung connection that
   would otherwise block init completely. It's not clear whether connect
   can hang, but if it can, that would be pretty bad.

   For the actual recover logic, see warn() and comments around setsignals().
   The only thing SIGALRM does is interrupting whatever call is blocking
   at the moment. */

void acceptctl(void)
{
	int fd;
	int gotcmd = 0;
	struct sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	struct itimerval it = {
		.it_interval = { 0, 0 },
		.it_value = { INITCTL_TIMEOUT, 0 }
	};

	/* initctlfd is SOCK_NONBLOCK */
	while((fd = accept(initctlfd, (struct sockaddr*)&addr, &addr_len)) > 0) {
		if(checkuser(fd)) {
			/* no need to bother with warnfd here */
			const char* denied = "Access denied\n";
			write(fd, denied, strlen(denied));
		} else {
			gotcmd = 1;
			setitimer(ITIMER_REAL, &it, NULL);
			readcmd(fd);
		} close(fd);
	} if(gotcmd) {
		/* disable the timer in case it has been set */
		it.it_value.tv_sec = 0;
		setitimer(ITIMER_REAL, &it, NULL);
	}

	state &= ~S_INITCTL;
}

/* Telinit socket, especially ANS socket, lacks any protection against
   non-root access. Which allows a simple kind-of-DoS attack: flooding
   the socket with connect() requests.
   It takes more effort for init to reply than it does for the attacker
   to connect, so it's init who suffers.

   A simple solution could be replying as fast as possible, but it's not
   possible. Even in the best case it's going to take more syscalls than
   a connection attempt.

   The only real protection is closing the listening socket.

   Which means, there's no point in trying to reply fast. We can safely
   log the attempt in warn(), and we can even send "Access denied" back
   instead of just closing the socket which is unbelievably polite. */

int checkuser(int fd)
{
	struct ucred cred;
	socklen_t credlen = sizeof(cred);

	if(getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &credlen))
		retwarn(-1, "cannot get peer credentials");
#ifdef DEVMODE
	if(cred.uid != getuid())
		retwarn(-1, "non-owner access, uid %i", cred.uid);
#else
	if(cred.uid)
		retwarn(-1, "non-root access, uid %i", cred.uid);
#endif
	return 0;
}

/* Typical command length here is *way* below atomic send limit. */

void readcmd(int fd)
{
	int rb;
	bss char cbuf[CMDBUF];

	if((rb = read(fd, cbuf, CMDBUF-1)) < 0)
		retwarn_("recvmsg failed: %m");
	if(rb >= CMDBUF)
		retwarn_("recvmsg returned bogus data");
	cbuf[rb] = '\0';

	warnfd = fd;
	parsecmd(cbuf);
	warnfd = 2;
}
