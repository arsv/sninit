#include <sys/types.h>
#include <fcntl.h>
#define _GNU_SOURCE
#include <poll.h>

typedef unsigned int nfds_t;

int __ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask, long sigsetsize);
int __libc_ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask) {
  return __ppoll(fds, nfds, timeout, sigmask, _NSIG/8);
}
int ppoll(struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const sigset_t *sigmask)
__attribute__((weak,alias("__libc_ppoll")));
