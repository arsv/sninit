#include <string.h>
#include "init.h"
#include "init_conf.h"
#include "scope.h"

/* Entry flags */

#define MAXCLS 8

#define R0 (1<<0)
#define R1 (1<<1)
#define R2 (1<<2)
#define R7 (1<<7)
#define R8 (1<<8)
#define R9 (1<<9)

#define Rx0	~R0
#define Rx012	(PRIMASK & ~(R0 | R1 | R2))
#define R789	(R7 | R8 | R9)
#define Rx789	(PRIMASK & ~(R7 | R8 | R9))

static struct entrytype {
	char key[MAXCLS];
	short rlvl;
	short flags;
} entypes[] = {
	{ "",		Rx012,	C_DOF | C_DTF },
	{ "resp",	Rx012,	C_HUSH },
	{ "slow",	Rx012,	C_DOF | C_DTF },
	{ "fast",	Rx012,	C_DOF },
	{ "wait",	Rx0,	C_ONCE | C_WAIT },
	{ "once",	Rx0,	C_ONCE },
	{ "last",	Rx0,	C_WAIT | C_DOF | C_DTF },
	{ "down",	R0,	C_ONCE | C_WAIT },
	{ "wake",	Rx789,	C_ONCE },
	{ "fall",	R789,	C_ONCE },
	/* not terminated */
};

local int applyentrytype(struct fileblock* fb, struct initrec* entry, const char* type)
{
	struct entrytype* et;
	struct entrytype* end = entypes + sizeof(entypes)/sizeof(*entypes);

	for(et = entypes; et < end; et++)
		if(!strcmp(type, et->key))
			break;
	if(et >= end)
		retwarn(-1, "%s:%i: unknown mode %s", fb->name, fb->line, type);

	entry->flags = et->flags;
	entry->rlvl = et->rlvl;
	return 0;
}

local int applyrunlevels(struct fileblock* fb, struct initrec* entry, const char* levels)
{
	const char* p;
	int rlvl = 0;

	for(p = levels; *p; p++)
		switch(*p) {
			case '0' ... '9':
				rlvl |= (1 << (*p - '0'));
				break;
			case 'a' ... 'f':
				rlvl |= (1 << (*p - 'a' + 0x0A));
				break;
			default:
				retwarn(-1, "%s:%i: bad runlevel char %c", fb->name, fb->line, *p);
		}

	if(rlvl & PRIMASK)
		entry->rlvl = rlvl;
	else if(rlvl & SUBMASK)
		entry->rlvl = (entry->rlvl & PRIMASK) | rlvl;

	return 0;
}

export int setrunflags(struct fileblock* fb, struct initrec* entry, char* type)
{
	/* split the field into type proper and optional runlevels part */
	char* colon = strchr(type, ':');
	if(colon) *colon = '\0';

	if(applyentrytype(fb, entry, type))
		return -1;
	if(!colon)
		return 0;
	return applyrunlevels(fb, entry, colon + 1);
}
