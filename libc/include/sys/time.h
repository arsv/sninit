#ifndef TIME_H
#define TIME_H

#include <bits/stat.h>

#define	ITIMER_REAL	0
#define	ITIMER_VIRTUAL	1
#define	ITIMER_PROF	2

typedef long suseconds_t;

struct timeval {
  time_t tv_sec;	/* seconds */
  suseconds_t tv_usec;	/* microseconds */
};

struct itimerval {
  struct timeval it_interval;	/* timer interval */
  struct timeval it_value;	/* current value */
};

int setitimer(int which, const struct itimerval *newval, struct itimerval *oldval);

#endif
