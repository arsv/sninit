#define _GNU_SOURCE
#include <sys/types.h>
#include <syscall.h>
#include "sys.h"

/* This should only be needed with glibc, other libc implementations
   provide proper direct getdents syscall. */

struct dirent64;

int getdents64(int fd, struct dirent64 *dirp, size_t count)
{
	return syscall3(__NR_getdents64, fd, (long)dirp, count);
}
