#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

struct initrec I0 = { .name = "mount" };
struct initrec I1 = { .name = "procps" };
struct initrec I2 = { .name = "" };
struct initrec I3 = { .name = "ftpd" };
struct initrec I4 = { .name = "httpd" };
struct initrec I5 = { .name = "named" };

struct initrec* IT0[] = { NULL, &I0, &I1, &I2, &I3, &I4, &I5, NULL };

struct config CF0 = {
	.inittab = IT0 + 1,
	.initnum = sizeof(IT0)/sizeof(void*) - 2
};

struct initrec* IT1[] = { NULL, NULL };

struct config CF1 = {
	.inittab = IT1 + 1,
	.initnum = sizeof(IT1)/sizeof(void*) - 2
};

struct config* cfg;

int main(void)
{
	cfg = &CF0;
	ASSERT(findentry("procps") == &I1);
	ASSERT(findentry("httpd") == &I4);
	ASSERT(findentry("blargh") == NULL);

	/* empty inittab; not likely to affect anything, but who knows */
	cfg = &CF1;
	ASSERT(findentry("httpd") == NULL);

	/* findentry is allowed to segfault if cfg->inittab == NULL */

	return 0;
}
