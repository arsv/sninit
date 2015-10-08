#include <errno.h>
#include <sys/reboot.h>

/* Numeric values as reported by qemu-i386 -strace:

   LINUX_REBOOT_MAGIC1: -18751827
   LINUX_REBOOT_MAGIC2: 672274793
   RB_HALT_SYSTEM: -839974621

   The call *should* fail with EACCESS. */

int main(void)
{
	if(reboot(RB_HALT_SYSTEM))
		return 0;
	else
		return -errno;
};
