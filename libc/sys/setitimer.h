#include <syscall.h>

inline static long syssetitimer(int which, struct itimerval* itv, struct itimerval* old)
{
	return syscall3(__NR_setitimer, which, (long)itv, (long)old);
}
