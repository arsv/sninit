#include <sys/types.h>
#include <sys/time.h>

struct tm* gmtime_r(const time_t* t, struct tm* r);
