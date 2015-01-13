#ifndef SYS_RESOURCE_H
#define SYS_RESOURCE_H

#include <bits/rlimit.h>

#define PRIO_PROCESS	0
#define PRIO_PGRP	1
#define PRIO_USER	2

struct rlimit {
  unsigned long	rlim_cur;
  unsigned long	rlim_max;
};

typedef int id_t;

int getrlimit (int resource, struct rlimit *rlim);
int setrlimit (int resource, const struct rlimit *rlim);

int getpriority(int which, id_t who);
int setpriority(int which, id_t who, int prio);

typedef unsigned long rlim_t;

#endif
