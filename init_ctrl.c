#define _GNU_SOURCE
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "sys.h"
#include "init.h"
#include "scope.h"

/* Init keeps an open unix(7) socket for telinit to connect to.
   The socket is SOCK_STREAM, to allow bi-directional communication
   and in particular arbitrary output from init.

   The actual command processing happens in init_cmds.c, the code
   here only receives them. */

int initctlfd = 0;

static int ctltime = 0;
static int ctlcount = 0;

static char ctlbuf[CMDBUF];

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
   instead of just closing the socket. */

static int checkuser(int fd)
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

/* Initctl is closed once there are more than N failed attempts
   in the last M seconds. To reopen the socket, root can kill -HUP 1
   at any point, preferably after checking syslog and kicking
   the abuser out of the system. */

static int checkthrottle(void)
{
	if(ctltime + THROTTLETIME < passtime) {
		ctlcount = 0;
		ctltime = passtime;
	}
	return (++ctlcount > THROTTLECOUNT);
}

/* Typical command length here is *way* below atomic send limit. */

static void readcmd(int fd)
{
	int rb;

	if((rb = read(fd, ctlbuf, CMDBUF-1)) < 0)
		retwarn_("recvmsg failed: %m");
	if(rb >= CMDBUF)
		retwarn_("recvmsg returned bogus data");
	ctlbuf[rb] = '\0';

	warnfd = fd;
	parsecmd(ctlbuf);
	warnfd = 2;
}

/* This gets called during startup, and also in case init gets SIGHUP. */

int setinitctl(void)
{
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = INITCTL
	};

	/* This way readable "@initctl" can be used for reporting below,
	   and config.h looks better too. */
	if(addr.sun_path[0] == '@')
		addr.sun_path[0] = '\0';

	/* we're not going to block for connections, just accept whatever
	   is already there; so it's SOCK_NONBLOCK */
	const int flags = SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
	if((initctlfd = socket(AF_UNIX, flags, 0)) < 0)
		retwarn(-1, "Can't create control socket: %m");

	if(bind(initctlfd, (struct sockaddr*)&addr, sizeof(addr)))
		gotowarn(close, "Can't bind %s: %m", INITCTL)
	else if(listen(initctlfd, 1))
		gotowarn(close, "listen() failed: %m");

	return 0;

close:
	close(initctlfd);
	initctlfd = -1;
	return -1;
}

/* We've got a pending connection on initctlfd, ppoll tells us.
   Accept it, and handle whatever command is there.

   Only one command is accepted for each connection. To send more, telinit
   must re-connect. This is to avoid implementing half-duplex communication
   here in init; instead, kernel-side connection state is used to manage
   data direction.

   Alarm (setitimer) is needed here to force-reset a hung connection that
   would otherwise block init completely.

   For the recovery logic, see warn() and comments around setsignals().
   In short, all SIGALRM does is interrupting whatever call is blocking
   at the moment, the rest is normal handling of negative read() or write()
   return. */

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
	while((fd = accept(initctlfd, (struct sockaddr*)&addr, &addr_len)) > 0)
	{
		int nonroot = checkuser(fd);

		if(nonroot) {
			/* no need to bother with warnfd here */
			const char* denied = "Access denied\n";
			write(fd, denied, strlen(denied));
		} else {
			gotcmd = 1;
			setitimer(ITIMER_REAL, &it, NULL);
			readcmd(fd);
		}

		close(fd);

		if(nonroot && checkthrottle()) {
			close(initctlfd);
			initctlfd = -1;
			break;
		}
	} if(gotcmd) {
		/* disable the timer in case it has been set */
		it.it_value.tv_sec = 0;
		setitimer(ITIMER_REAL, &it, NULL);
	}
}
