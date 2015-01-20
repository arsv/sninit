#ifndef SYS_FSUID_H
#define SYS_FSUID_H

#include <sys/types.h>

int setfsuid(uid_t uid);
int setfsgid(gid_t gid);

#endif
