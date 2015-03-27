#include <string.h>
#include "init.h"

/* This tiny function is called from two wildly different places,
   so to keep dependencies sane it was moved to its own file.

   init_conf uses it when transferring pids
   init_cmds uses it to start/stop entries

   Note: find an entry from the primary inittab, cfg->inittab, the one that
   initpass uses. Which may or may not be located in cfgblock. */

extern struct config* cfg;

struct initrec* findentry(const char* name)
{
	struct initrec *p, **pp;

	for(pp = cfg->inittab; (p = *pp); pp++)
		if(p->name && !strcmp(p->name, name))
			return p;

	return NULL;
}
