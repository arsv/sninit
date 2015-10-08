#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <stdarg.h>
#include <stdio.h>

/* Stub syslog server, with STREAM socket. Do not use it, except for checking
   init's SOCK_STREAM capability. It uses a dirty alarm() trick to avoid muxing
   inputs or forking while still retaining limited ability to handle several
   connections. */

#define BUF 1024
#define HWM 900
#define UNIX_PATH_MAX 108

static int opensocket(const char* name);
static void handleconn(int infd, int outfd);
static int shiftwrite(int fd, char* buf, int tag, int len);
static void die(const char* fmt, ...) __attribute__((noreturn));
static void sighandler(int sig);

struct sigaction sa = {
	.sa_handler = sighandler,
	.sa_flags = 0,
};

int main(int argc, char** argv)
{
	int sfd;
	int cfd;
	int outfd = 1;

	if(argc < 2 || argc > 3)
		die("Usage: slogd socket-name [log-file]\n");
	if(argc > 2)
		if((outfd = open(argv[2], O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
			die("Cannot open %s: %m\n", argv[2]);

	if(sigaction(SIGALRM, &sa, NULL))
		die("sigaction: %m\n");

	sfd = opensocket(argv[1]);

	while((cfd = accept(sfd, NULL, NULL)) > 0) {
		alarm(1);
		handleconn(cfd, outfd);
		close(cfd);
		alarm(0);
	}

	close(cfd);

	return 0;
}

void sighandler(int sig)
{
	printf("sig %i\n", sig);
}

int opensocket(const char* name)
{
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un addr = { .sun_family = AF_UNIX };

	memset(addr.sun_path, 0, UNIX_PATH_MAX);
	strncpy(addr.sun_path, name, UNIX_PATH_MAX - 1);

	unlink(addr.sun_path);

	if(fd < 0)
		die("cannot create socket: %m\n");

	if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		die("cannot bind %s: %m\n", addr.sun_path);

	if(listen(fd, 0) < 0)
		die("listen: %m\n");

	return fd;
}

void handleconn(int infd, int outfd)
{
	char buf[BUF] = "syslog: ";
	int tag = strlen(buf);

	char* rdbuf = buf + tag;
	int rdlen = BUF - tag;
	int rdptr = 0, rdnew;
	int rd;

	while((rd = read(infd, rdbuf + rdptr, rdlen - rdptr)) > 0) {
		rdptr += rd;
		rdnew = shiftwrite(outfd, buf, tag, rdptr);

		if(rdnew > HWM)
			return;
	};

	shiftwrite(outfd, buf, tag, rdptr);
}

static int shiftwrite(int outfd, char* buf, int tag, int len)
{
	while(1) {
		char* end = buf + tag + len;
		char* p = buf + tag;

		while(*p && p < end) p++;

		if(p >= end) break;

		*p++ = '\n';
		write(outfd, buf, p - buf);

		if((len = buf + len - p) <= 0)
			break;

		memmove(buf + tag, p, len);
	}
	return len;
}

void die(const char* fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	_exit(-1);
}
