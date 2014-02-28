/* Stubs for init_conf entry points. */

#include "init.h"

int configure(int strict)
{
	if(strict)
		warn("init is not configurable");
	return -1;
}

void setnewconf(void)
{

}
