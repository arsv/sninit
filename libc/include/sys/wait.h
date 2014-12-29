#ifndef WAIT_H
#define WAIT_H

#include <sys/types.h>

#define WNOHANG		0x00000001

#define WEXITSTATUS(status)	(((status) & 0xff00) >> 8)
#define WTERMSIG(status)	((status) & 0x7f)

#define WIFEXITED(status)	(WTERMSIG(status) == 0)
#define WIFSIGNALED(status)	(!WIFSTOPPED(status) && !WIFEXITED(status))
#define WIFSTOPPED(status)	(((status) & 0xff) == 0x7f)

pid_t waitpid(pid_t pid, int *status, int options);

#endif
