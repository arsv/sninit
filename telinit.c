#define	_GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"

#define BUF 256

static char cmdname(const char* cmd);
static int runcmd(const char* cmd);
static void die(const char* msg, ...);

static int rlcmd(char cmd, char* rl);
static int noargcmd(char cc);
static int argrepeatcmd(char cc, int argc, char** arg);

int main(int argc, char** argv)
{
	char cc = 0;

	if(argc < 2)
		die("Usage: telinit cmd [args]\n");

	cc = cmdname(argv[1]);
	if(!cc)
		die("telinit: unknown command %s\n", argv[1]);

	switch(cc) {
		case '=':
			return rlcmd(cc, argv[1]);
		case '+':
		case '-':
			return rlcmd(cc, argv[1]+1);
		case 'd':
		case 'e':
			return argrepeatcmd(cc, argc, argv);
		default:
			return noargcmd(cc);
	}

	return 0;
}

static struct cmdrec {
	char* name;
	char cmd;
} cmdtbl[] = {
	{ "reload",	'r' },
	{ "start",	'e' },
	{ "stop",	'd' },
	{ "enable",	'e' },
	{ "disable",	'd' },
	{ "poweroff",	'P' },
	{ "halt",	'H' },
	{ "reboot",	'R' },
	{ "sleep",	'8' },
	{ "suspend",	'9' },
	{ NULL }
};

static char cmdname(const char* cmd)
{
	struct cmdrec* p;

	if(*cmd >= '0' && *cmd <= '9')
		return '=';
	if(*cmd == '-' || *cmd == '+')
		return *cmd;
	if(*cmd && cmd[1] == '\0')
		return *cmd;

	for(p = cmdtbl; p->name; p++)
		if(!strcmp(cmd, p->name))
			return p->cmd;

	return 0;
}

static int cmdbuflen(int argc, char** argv)
{
	int cl, cm = 1;
	int i;

	for(i = 2; i < argc; i++) {
		cl = strlen(argv[i]) + 2;
		if(cl > cm) cm = cl;
	}

	return cm + 2;	/* command code, arg, newline */
}

static int rlcmd(char cmd, char* rl)
{
	int len = strlen(rl);
	char buf[len + 4];

	buf[0] = cmd;
	strncpy(buf + 1, rl, len);
	buf[len+2] = '\0';
	buf[len+3] = '\0';

	return runcmd(buf);
}

static int noargcmd(char cc)
{
	char buf[3];
	buf[0] = cc;
	buf[1] = '\0';
	buf[2] = '\0';
	return runcmd(buf);
}

static int argrepeatcmd(char cc, int argc, char** argv)
{
	int cm = cmdbuflen(argc, argv);
	char buf[cm+1];
	int r = -1;
	int i;

	for(i = 2; i < argc; i++) {
		buf[0] = cc;
		strcpy(buf+1, argv[i]);
		if((r = runcmd(buf)))
			return r;
	};
	
	return r;
}

static int opensocket(void)
{
	int fd;
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = INITCTL
	};

	if(addr.sun_path[0] == '@')
		addr.sun_path[0] = '\0';

	fd = socket(AF_UNIX, SOCK_STREAM, 0);	
	if(fd < 0)
		die("Can't create socket: %m\n");
	if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)))
		die("Can't connect to %s: %m\n", INITCTL);

	return fd;
}

static int sendcmd(int fd, const char* cmd)
{
	char mbuf[CMSG_SPACE(sizeof(struct ucred))];
	struct iovec iov[1] = { {
		.iov_base = (char*)cmd,	/* sendmsg isn't going to change it */
		.iov_len = strlen(cmd)	
	} };
	struct msghdr mhdr = {
		.msg_name = NULL,
		.msg_iov = iov,
		.msg_iovlen = 1,
		.msg_control = mbuf,
		.msg_controllen = sizeof(mbuf),
		.msg_flags = 0
	};
	struct cmsghdr *cmsg;
	struct ucred cred = {
		.pid = getpid(),
		.uid = geteuid(),
		.gid = getegid()
	};

	memset(mbuf, 0, sizeof(mbuf));	/* erase the whole buffer, to keep valgrind happy
					   and the message clean of any stack noise */
	cmsg = CMSG_FIRSTHDR(&mhdr);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_CREDENTIALS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(cred));
	memcpy((int *)CMSG_DATA(cmsg), &cred, sizeof(cred));

	if(sendmsg(fd, &mhdr, 0) < 0)
		die("sendmsg failed: %m\n");
	
	return 0;
}

static void recvreply(int fd)
{
	char buf[BUF];
	int rr;

	while((rr = read(fd, buf, BUF)) > 0)
		write(2, buf, rr);
}

static int runcmd(const char* cmd)
{
	int fd;

	fd = opensocket();
	sendcmd(fd, cmd);
	shutdown(fd, SHUT_WR);
	recvreply(fd);
	close(fd);

	return 0;
};

static void die(const char* msg, ...)
{
	va_list ap;
	char buf[256];

	va_start(ap, msg);
	vsnprintf(buf, 256, msg, ap);
	va_end(ap);

	write(2, buf, strlen(buf));
	_exit(-1);
}
