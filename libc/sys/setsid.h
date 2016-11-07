#include <bits/syscall.h>

inline static long syssetsid(void)
{
	return syscall0(__NR_setsid);
}
