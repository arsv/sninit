#include "../init.h"
#include "_test.h"

int currlevel;
int nextlevel;
struct config* cfg;

/* we are going to use bundled version in any case,
   so no point in including stdio.h */
extern int vsnprintf(char* buf, size_t len, const char* fmt, va_list ap);

#define BUFLEN 1024
char warnbuf[BUFLEN];

int warn(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(warnbuf, BUFLEN, fmt, ap);
	va_end(ap);
	return 0;
}

int levelmatch(struct initrec* p, int level)
{
	return 1;
};

extern void dumprec(struct initrec* p, int namewidth, int pidwidth);

struct initrec w1 = { .name = "mount",    .flags = C_ONCE,    .pid = 23,
	.argv = { "/sbin/mount", "-a", NULL } };
struct initrec s1 = { .name = "cpuburnd", .flags = 0,         .pid = 34,
	.argv = { "/sbin/cpuburnd", "-N", "-D", NULL } };
struct initrec s2 = { .name = "qrd",      .flags = P_MANUAL,  .pid = 0,
	.argv = { "/sbin/qrd", "-f", "/etc/qrd.conf", NULL } };
struct initrec s3 = { .name = "cpuburnd", .flags = P_SIGSTOP, .pid = 3456, 
	.argv = { "/sbin/cpuburnd", "-N", "-D", NULL } };

/*           name        +pid     command                       */
/*           01234567890123456789012345678901234567890123456789 */
/*                   vvvv     vvvv                              */
char* sw1 = "mount        23      /sbin/mount -a";
char* ss1 = "cpuburnd     34      /sbin/cpuburnd -N -D";
char* ss2 = "qrd          -       /sbin/qrd -f /etc/qrd.conf";
char* ss3 = "cpuburnd    *3456    /sbin/cpuburnd -N -D";
/*                   ^^^^     ^^^^                          */
/*           01234567890123456789012345678901234567890123456789 */

/* Not testing ... ending here, it depends on config.h
   and there are separate tests for that in t_joincmd. */

const int nw = 8;
const int pw = 4;

#define TEST(rec, nw, pw, exp) {\
	dumprec(&rec, nw, pw);\
	S(warnbuf, exp);\
}

int main(void)
{
	TEST(w1, nw, pw, sw1);
	TEST(s1, nw, pw, ss1);
	TEST(s2, nw, pw, ss2);
	TEST(s3, nw, pw, ss3);
	return 0;
}
