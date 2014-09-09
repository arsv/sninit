#ifndef SYS_RESOURCE_H
#define SYS_RESOURCE_H

#include <bits/rlimit.h>

struct rlimit {
  unsigned long	rlim_cur;
  unsigned long	rlim_max;
};

int getrlimit (int resource, struct rlimit *rlim);
int setrlimit (int resource, const struct rlimit *rlim);

typedef unsigned long rlim_t;

#endif
