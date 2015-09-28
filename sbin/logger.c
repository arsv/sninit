#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>

/* A simple syslog client. Works with DGRAM and STREAM sockets,
   the same way init and dietlibc syslog(3) implementations do.

   No timestamp is prepended to the message (init's "nots" mode).
   The tag is set to 29 (LOG_NOTICE | LOG_DAEMON) to match init's. */

#define BUF 1024
#define UNIX_PATH_MAX 108

static int mergemsg(char* buf, int len, char** argv, int argc);
static int trysocket(char* name, int type);
static void die(const char* fmt, ...) __attribute__((noreturn));

int main(int argc, char** argv)
{
	int fd = -1;
	int socktype;
	char buf[BUF] = "<29> logger: ";

	if(argc < 3)
		die("Usage: logger socket-name message\n");

	if(fd < 0)
		fd = trysocket(argv[1], socktype = SOCK_DGRAM);
	if(fd < 0)
		fd = trysocket(argv[1], socktype = SOCK_STREAM);
	if(fd < 0)
		die("Cannot connect to syslog\n");

	int tag = strlen(buf);
	int len = mergemsg(buf + tag, BUF - tag - 1, argv + 2, argc - 2);

	if(socktype == SOCK_STREAM)
		buf[tag + len++] = '\0';

	int ret = send(fd, buf, tag + len, 0);

	if(ret < 0)
		die("send: %m\n");
	else if(ret < len)
		die("sent %i bytes out of %i\n", ret, len);

	return (ret == len) ? 0 : -1;
}

static int trysocket(char* name, int type)
{
	int fd;
	struct sockaddr_un addr = { .sun_family = AF_UNIX };

	memset(addr.sun_path, 0, UNIX_PATH_MAX);
	strncpy(addr.sun_path, name, UNIX_PATH_MAX - 1);

	if((fd = socket(AF_UNIX, type, 0)) < 0)
		return -1;

	if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)))
		return -1;

	return fd;
}

static int mergemsg(char* buf, int rem, char** argv, int argc)
{
	int i;
	int len;
	int ret = 0;

	rem--;
	for(i = 0; i < argc; i++) {
		len = strlen(argv[i]);
		memcpy(buf + ret, argv[i], len < rem ? len : rem);
		rem -= len;
		ret += len;
		if(rem > 0 && i < argc-1) buf[ret++] = ' ';
	}
	buf[ret] = '\0';

	return ret;
}

void die(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	_exit(-1);
}
