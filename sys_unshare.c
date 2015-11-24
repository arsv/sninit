#define _GNU_SOURCE
#include <sys/syscall.h>
#include <sched.h>

/* For non-patched dietlibc. Do not forget to include -lcompat in LIBS. */

int unshare(int flags)
{
	return syscall(__NR_unshare, flags);
}
