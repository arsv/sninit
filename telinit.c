#define	_GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>

#include "config.h"

#define ERR ((void*) -1)

static int runcmd(const char* cmd);
static void die(const char* msg, const char* arg);

static struct cmdrec {
	char* name;
	char cc;
	char arg;
} cmdtbl[] = {
	{ "reload",	'r', 0 },
	{ "start",	'e', 1 },
	{ "stop",	'd', 1 },
	{ "poweroff",	'P', 0 },
	{ "halt",	'H', 0 },
	{ "reboot",	'R', 0 },
	{ "sleep",	'p', 0 },
	{ "suspend",	'u', 0 },
	{ "?",		'?', 0 },
	{ NULL }
};

// .bss is zero-initialized, so no need to call memset() later
static char buf[NAMELEN+2];

int main(int argc, char** argv)
{
	struct cmdrec* cr = NULL;

	int arg = 0;
	char* ptr = buf;
	char* cmd = argv[1];
	int i;

	if(argc < 2)
		die("Usage: telinit cmd [args]", NULL);

	if(*cmd >= '0' && *cmd <= '9' && !*(cmd+1)) {
		buf[0] = '=';
		buf[1] = *cmd;
	} else if(*cmd == '+' || *cmd == '-' || *cmd == '=') {
		ptr = cmd;
	} else {
		for(cr = cmdtbl; cr->name; cr++) {
			if(!*(cmd+1) && *cmd == cr->cc)
				break;
			if(!strcmp(cmd, cr->name))
				break;
		}
		if(cr->name) {
			buf[0] = cr->cc;
			arg = cr->arg;
		} else {
			die("Unknown command ", cmd);
		}
	}

	if(!arg)
		runcmd(ptr);
	else for(i = 2; i < argc; i++) {
		strncpy(buf + 1, argv[i], sizeof(buf)-2);
		runcmd(buf);
	}

	return 0;
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
		die("Can't create socket: ", ERR);
	if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)))
		die("Can't connect to " INITCTL ": ", ERR);

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
		die("sendmsg failed: ", ERR);
	
	return 0;
}

static void recvreply(int fd)
{
	char buf[100];
	int rr;

	while((rr = read(fd, buf, sizeof(buf))) > 0)
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

static void die(const char* msg, const char* arg)
{
	char buf[256];
	int len = strlen(msg);
	int max = sizeof(buf) - 2;

	strncpy(buf, msg, max);

	if(arg == ERR)
		arg = strerror(errno);
	if(arg)
		strncpy(buf + len, arg, max - len);

	len = strlen(buf);
	buf[len++] = '\n';

	write(2, buf, len);
	_exit(-1);
}
