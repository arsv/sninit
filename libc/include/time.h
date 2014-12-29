#include <sys/types.h>
#include <sys/time.h>

/* These seem to be arch-independent, so let's put them
   here instead of using ARCH/bits/time.h */
#define CLOCK_MONOTONIC		1
#define CLOCK_BOOTTIME		7

struct tm* gmtime_r(const time_t* t, struct tm* r);
int clock_gettime(clockid_t clk_id, struct timespec *tp);
int nanosleep(const struct timespec *req, struct timespec *rem);
