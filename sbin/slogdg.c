#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>

/* Stub syslog server, with DGRAM socket. */

#define LEN 1024
#define UNIX_PATH_MAX 108

static int opensocket(const char* name);
static void die(const char* fmt, ...) __attribute__((noreturn));

int main(int argc, char** argv)
{
	char buf[LEN+2] = "syslog: ";
	int tag = strlen(buf);
	int len;
	int sockfd;
	int outfd = 1;

	if(argc < 2 || argc > 3)
		die("Usage: slogd socket-name [log-file]\n");
	if(argc > 2)
		if((outfd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
			die("Cannot open %s: %m\n", argv[2]);

	sockfd = opensocket(argv[1]);

	while((len = recv(sockfd, buf + tag, LEN - tag, 0)) > 0) {
		buf[tag + len] = '\n';
		write(outfd, buf, tag + len + 1);
	}

	close(sockfd);

	return 0;
}

int opensocket(const char* name)
{
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	struct sockaddr_un addr = { .sun_family = AF_UNIX };

	memset(addr.sun_path, 0, UNIX_PATH_MAX);
	strncpy(addr.sun_path, name, UNIX_PATH_MAX - 1);

	unlink(addr.sun_path);

	if(fd < 0)
		die("cannot create socket: %m\n");

	if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		die("cannot bind %s: %m\n", addr.sun_path);

	return fd;
}

void die(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	_exit(-1);
}
