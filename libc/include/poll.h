#ifndef _POLL_H
#define _POLL_H

#include <signal.h>
#include <sys/time.h>

#define POLLIN	(1<<0)
#define POLLPRI	(1<<1)
#define POLLOUT (1<<2)
#define POLLERR (1<<3)
#define POLLHUP (1<<4)

struct pollfd {
  int fd;
  short events;
  short revents;
};

int ppoll(struct pollfd *fds, unsigned int nfds, const struct timespec *timeout, const sigset_t *sigmask);

#endif
