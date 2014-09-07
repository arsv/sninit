#include <sys/reboot.h>

#define LINUX_REBOOT_MAGIC1	0xfee1dead
#define LINUX_REBOOT_MAGIC2	672274793

int __reboot(unsigned int magic1, unsigned int magic2, int cmd);

int reboot(int cmd)
{
  return __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, cmd);
}
