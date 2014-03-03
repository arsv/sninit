/* Null configuration, replaces normal init_conf_* routines with stubs.
   This file should be linked *instead* of init_conf_* when run-time
   reconfiguration is not needed. Static inittab must be built in,
   otherwise init will have no config to run with. */

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
