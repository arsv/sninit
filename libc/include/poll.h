#ifndef POLL_H
#define POLL_H

#include <signal.h>
#include <sys/time.h>
#include <bits/poll.h>

struct pollfd {
  int fd;
  short events;
  short revents;
};

int ppoll(struct pollfd *fds, unsigned int nfds, const struct timespec *timeout, const sigset_t *sigmask);

#endif
