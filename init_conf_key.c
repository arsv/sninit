#include <string.h>
#include "init.h"
#include "init_conf.h"
#include "scope.h"

/* Second inittab field parsing.

         mount    W1+    /sbin/mount -a
                  ~~~
   The leading letter determines entry's flags, the rest is runlevel mask.
   Plus sign extends the last primary runlevel: 3+ = 3456789

   Empty runlevel mask makes no sense, so instead "R" = "R123456789" and
   "S+" = "R0123456789". This way "R+" will never be run, hm.

   Initdir entries here always have properly formatted type. */

/* The idea is that entypes[] should list all useful flag combos.
   Which is less that just all combos: DOF/DTF make no difference with ONCE,
   for instance, HUSH = not (DOF or DTF) and not ONCE, and there may be others.

   X inverts its runlevel mask. It looks better than having a separate
   invert-mask sign. Inverting ONCE|WAIT and inverting non-ONCE are not
   likely to be useful, so they are excluded. */

static struct entrytype {
	char key;
	short flags;
} entypes[] = {
	{ 'S', C_DOF | C_DTF },
	{ 'L', C_DOF | C_DTF | C_WAIT },
	{ 'F', C_DOF },
	{ 'T', C_HUSH | C_KILL },
	{ 'P', C_HUSH },
	{ 'W', C_ONCE | C_WAIT },
	{ 'R', C_ONCE },
	{ 'X', C_ONCE | C_INVERT },
	{ 0, 0 }
};

export int setrunflags(struct fileblock* fb, struct initrec* entry, char* type)
{
	char* p;
	int rlvl = 0;
	int last = 0;

	if(!*type) /* cannot risk *(type+1) below */
		retwarn(-1, "%s:%i: empty entry key", fb->name, fb->line);

	for(p = type + 1; *p; p++)
		switch(*p) {
			case '0' ... '9':
				rlvl |= (1 << (last = *p - '0'));
				break;
			case 'a' ... 'f':
				rlvl |= (1 << (*p - 'a' + 0xA));
				break;
			case '+':
				rlvl |= (PRIMASK << last) & PRIMASK;
				break;
			default:
				retwarn(-1, "%s:%i: bad runlevel specifier", fb->name, fb->line);
		}
	if(!(rlvl & PRIMASK))
		rlvl = (PRIMASK & ~1);

	struct entrytype* et;
	for(et = entypes; et->key; et++)
		if(*type == et->key)
			break;
	if(!et->key)
		retwarn(-1, "%s:%i: unknown entry type %c", fb->name, fb->line, *type);

	entry->flags = et->flags;
	entry->rlvl = rlvl;

	return 0;
}
