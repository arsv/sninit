#define	_GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>

#include "config.h"

#define ERR ((void*) -1)

/* Telinit sends commands to init via its control socket and reads init's
   output back. One command is sent per connection. In case there are
   multiple arguments, telinit sends one at a time and reconnects between
   sending them. There is no multiplexing of any sort, it's write-all and
   followed by read-all for each command. */

static int runcmd(const char* cmd);
static void die(const char* msg, const char* arg);

/* For convenience, one-letter init command codes (cc's) are given readable
   names within telinit. These should be kept in sync with init_cmds.c.
   Sleep commands are "telinit-only", they send generic level switch cc's. */

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

/* Init can only be controlled by root. This is enforced by sending
   user credentials in auxiliary data. Telinit could have checked it
   too, giving early non-root access error, but it's not done. First,
   the code is already in init, and second, there DEVMODE. */

static int sendcmd(int fd, const char* cmd)
{
	int len = strlen(cmd);

	if(write(fd, cmd, len) < 0)
		die("write failed: ", ERR);
	
	return 0;
}

/* The tricky part here is demuxing init output, which can be error
   messages to be sent to stderr, or pidof/list output which is stdout
   kind of data.

   Init has a very specific reply pattern, it's eiter all-error
   or all-non-error, so a simple # indicator at the start of init
   output is used to choose the fd.

   As a side effect, this also determines telinit return code.
   Non-empty error message means there was an error, which empty
   output or any #-output means everything went well. */

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
