#include <string.h>
#include "init.h"
#include "init_conf.h"

/* Second inittab field parsing.

         mount    W4*    /sbin/mount -a
                  ~~~
   The leading letter determines entry's flags, the rest is runlevel mask.
   Asterisk extends the last primary runlevel: 4* = 456789

   Empty runlevel mask makes no sense, so instead "R" = "R23456789"
   (see DEFAULTMASK) and "R*" = "R123456789".

   Initdir entries here always have a proper type string.
   In case none was specified in the file, a default one is used. */

/* The idea behind entry keys is that entypes[] should list all useful
   flag combos. Which is less that just all combos: FAST make no difference
   with ONCE, FAST implies HUSH, and so on.

   X inverts its runlevel mask. This looks better than having a separate
   invert-mask sign. Inverting ONCE|WAIT and inverting non-ONCE are not
   likely to be useful, so they are not implemented. */

static struct entrytype {
	char key;
	short flags;
} entypes[] = {
	{ 'S', 0 },
	{ 'L', C_WAIT },
	{ 'F', C_FAST },
	{ 'H', C_HUSH },
	{ 'T', C_FAST | C_HUSH | C_KILL },
	{ 'W', C_ONCE | C_WAIT },
	{ 'R', C_ONCE },
	{ 'X', C_ONCE | C_INVERT },
	{ 0, 0 }
};

int setrunflags(struct initrec* entry, const char* type)
{
	const char* p;
	int rlvl = 0;
	int last = 1;		/* R* entries translate to R12...9 */

	if(!*type) /* cannot risk *(type+1) below */
		retwarn(-1, "%s:%i: empty entry key", FBN, FBL);

	for(p = type + 1; *p; p++) switch(*p) {
		case '0' ... '9':
			rlvl |= (1 << (last = *p - '0'));
			break;
		case 'a' ... 'f':
			rlvl |= (1 << (*p - 'a' + 0xA));
			break;
		case '*':
			rlvl |= (PRIMASK << last) & PRIMASK;
			if(!*(p+1)) break;
			retwarn(-1, "%s:%i: no characters allowed past *", FBN, FBL);
		default:
			retwarn(-1, "%s:%i: bad runlevel specifier", FBN, FBL);
	} if(!(rlvl & PRIMASK))
		rlvl |= DEFAULTMASK;

	struct entrytype* et;
	for(et = entypes; et->key; et++)
		if(*type == et->key)
			break;
	if(!et->key)
		retwarn(-1, "%s:%i: unknown entry type %c", FBN, FBL, *type);

	entry->flags = et->flags;
	entry->rlvl = rlvl;

	return 0;
}
