#ifndef WAIT_H
#define WAIT_H

#include <sys/types.h>

#define WNOHANG		0x00000001

pid_t waitpid(pid_t pid, int *status, int options);

#endif
