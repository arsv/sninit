#ifndef __SYS_REBOOT_H__
#define __SYS_REBOOT_H__

#include <bits/syscall.h>
#include <bits/reboot.h>

inline static long sysreboot(int cmd)
{
	return syscall4(__NR_reboot, 0xfee1dead, 0x28121969, cmd, 0);
}

#endif
