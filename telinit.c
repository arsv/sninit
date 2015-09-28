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
	char cc;
	char arg;
	char name[10];
} cmdtbl[] = {
	{ 'c', 0, "reload",	},
	{ 'r', 1, "restart",	},
	{ 's', 1, "start",	},
	{ 't', 1, "stop",	},
	{ 'u', 1, "unstop",	},
	{ 'p', 1, "pause",	},
	{ 'h', 1, "hup",	},
	{ 'i', 1, "pidof",	},
	{ 'w', 1, "resume",	},
	{ 'P', 0, "poweroff",	},
	{ 'H', 0, "halt",	},
	{ 'R', 0, "reboot",	},
	{ '7', 0, "doze",	},
	{ '8', 0, "sleep",	},
	{ '9', 0, "suspend",	},
	{ '?', 0, "list",	},
	{  0  }
};

int main(int argc, char** argv)
{
	struct cmdrec* cr = NULL;
	char buf[NAMELEN+2];

	int hasarg = 0;
	char* ptr = buf;
	char* cmd = argv[1];
	char* cm1 = cmd + 1;
	int i;

	if(argc < 2)
		die("Usage: telinit cmd [args]", NULL);

	if((*cmd >= '0' && *cmd <= '9') || *cmd == '-' || *cmd == '+') {
		ptr = cmd;
	} else {
		for(cr = cmdtbl; cr->cc; cr++) {
			if(!*cm1 && *cmd == cr->cc)
				break;
			if(!strcmp(cmd, cr->name))
				break;
		} if(!cr->cc)
			die("Unknown command ", cmd);

		buf[0] = cr->cc;
		buf[1] = '\0';
		hasarg = cr->arg;
	}

	int ret = 0;

	if(!hasarg)
		ret = runcmd(ptr);
	else for(i = 2; i < argc; i++) {
		memset(buf + 1, 0, sizeof(buf)-1);
		strncpy(buf + 1, argv[i], sizeof(buf)-2);
		ret |= runcmd(buf);
	}

	return (ret ? 1 : 0);
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

static int recvreply(int fd)
{
	char buf[100];
	int rr;
	int out = 0;
	int ret = 0;
	int off = 0;

	while((rr = read(fd, buf, sizeof(buf))) > 0) {
		if(!out) {
			if(buf[0] == '#') {
				off = 1;
				out = 1;
			} else {
				ret = -1;
				out = 2;
			}
		} else if(off) off = 0;

		write(out, buf + off, rr - off);
	}

	return ret;
}

static int runcmd(const char* cmd)
{
	int fd;
	int r = 0;

	fd = opensocket();
	sendcmd(fd, cmd);
	shutdown(fd, SHUT_WR);
	r = recvreply(fd);
	close(fd);

	return r;
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
