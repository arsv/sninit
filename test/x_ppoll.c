#define _GNU_SOURCE
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

int main(void)
{
	struct pollfd pfd = { .fd = 0, .events = POLLIN, .revents = 0 };
	struct timespec pts = { 0, 500000000 };
	sigset_t st;

	sigemptyset(&st);

	int r = ppoll(&pfd, 1, &pts, &st);

	if(r < 0 && errno != EINTR)
		return -errno;
	else if(r >= 0)
		return 0;
	else
		return -1;
}
