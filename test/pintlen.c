#include "../init.h"
#include "_test.h"

int currlevel;
int nextlevel;
struct config* cfg;

int main(void)
{
	INTEQUALS(pintlen(1),     1);
	INTEQUALS(pintlen(12),    2);
	INTEQUALS(pintlen(123),   3);
	INTEQUALS(pintlen(1234),  4);
	INTEQUALS(pintlen(12345), 5);
	INTEQUALS(pintlen(0),     0);
	INTEQUALS(pintlen(-1),    0);
	return 0;
}
