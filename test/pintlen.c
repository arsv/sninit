#include "_test.h"

int currlevel;
int nextlevel;
struct config* cfg;

extern int pintlen(int n);

NOCALL(warn);
NOCALL(levelmatch);

#define Eqi(val, exp) Eq(val, exp, "%i")

int main(void)
{
	Eqi(pintlen(1), 1);
	Eqi(pintlen(12), 2);
	Eqi(pintlen(123), 3);
	Eqi(pintlen(1234), 4);
	Eqi(pintlen(12345), 5);
	Eqi(pintlen(0), 0);
	Eqi(pintlen(-1), 0);
	return 0;
}
