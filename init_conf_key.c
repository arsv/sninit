#include <string.h>
#include "init.h"
#include "init_conf.h"
#include "scope.h"

static struct entrytype {
	char key;
	char inv;
	short flags;
} entypes[] = {
	{ 'S', 0, C_DOF | C_DTF },
	{ 'L', 0, C_DOF | C_DTF | C_WAIT },
	{ 'F', 0, C_DOF },
	{ 'P', 0, C_HUSH },
	{ 'W', 0, C_ONCE | C_WAIT },
	{ 'R', 0, C_ONCE },
	{ 'X', 1, C_ONCE },
	{ 0, 0, 0 }
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
	entry->rlvl = et->inv ? ((~rlvl & PRIMASK) | (rlvl & SUBMASK)) : rlvl;

	return 0;
}
