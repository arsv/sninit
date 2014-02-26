/* Parsing inittab */

#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "init.h"
#include "init_conf.h"

extern struct memblock scratch;

extern int addinitrec(struct fileblock* fb, char* name, char* rlvl, char* flags, char* cmd, int exe);
extern int scratchenv(const char* string);
extern int readinitdir(struct fileblock* fb, const char* dir, int defrlvl, int strict);
extern int mmapfile(struct fileblock* fb, int maxlen);
extern int munmapfile(struct fileblock* fb);
extern int nextline(struct fileblock* f);

int parseinitline(struct fileblock* fb, int strict);
int parsedirline(struct fileblock* fb, char* line, int strict);

extern int setrunlevels(struct fileblock* fb, unsigned short* rlvl, char* runlevels);

/* Top-level inittab format */
/* Strict means bail out on errors immediately; with strict=0, it should continue
   as far as possible, assuming it's initial configuration with no fallback. */
int readinittab(const char* file, int strict)
{
	int ret = -1;
	struct fileblock fb = {
		.name = file,
		.line = 0,
		.rlvl = (PRIMASK & ~1)
	};

	if(mmapfile(&fb, -MAXFILE))
		return -1;

	while(nextline(&fb))
		if((ret = parseinitline(&fb, strict)) && strict)
			break;

	munmapfile(&fb);

	return strict ? ret : 0;
}

/* Parse current line from fb, the one marked by fb->ls and fb->le. */
int parseinitline(struct fileblock* fb, int strict)
{
	char* p;
	char *name, *runlvl, *flags;
	char* l = fb->ls;

	if(!*l || *l == '#')
		/* empty or comment line */
		return 0;

	if(*l == '@')
		return parsedirline(fb, l + 1, strict);

	p = strpbrk(l, ":=");
	if(!p)
		retwarn(-1, "%s:%i: bad line", fb->name, fb->line);
	else if(*p == '=')
		return (scratchenv(l) >= 0 ? 0 : -1);

	*(p++) = '\0'; name = l;
	runlvl = strsep(&p, ":");
	flags = strsep(&p, ":");
	if(!flags || !runlvl)
		return -1;

	return addinitrec(fb, name, runlvl, flags, p, 0);
}

int parsedirline(struct fileblock* fb, char* l, int strict)
{
	char* p = strpbrk(l, ":");
	char* name = p ? p + 1 : l + 1;
	char* runlvl = p ? l + 1 : NULL;
	*p = '\0';

	unsigned short defrlvl = 0;
	if(setrunlevels(fb, &defrlvl, runlvl))
		return -1;

	return readinitdir(fb, name, defrlvl, strict);
}
