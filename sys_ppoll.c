#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sys/poll.h>

/* For non-patched dietlibc. Do not forget to include -lcompat in LIBS. */

int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout_ts, const sigset_t *sigmask)
{
	return syscall(__NR_ppoll, fds, nfds, timeout_ts, sigmask, _NSIG/8);
}
