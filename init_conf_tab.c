/* Parsing inittab */

#include <string.h>
#include "init.h"
#include "init_conf.h"

extern struct memblock newblock;

extern int addinitrec(struct fileblock* fb, char* code, char* name, char* cmd, int exe);
extern int addenviron(const char* string);
extern int setrunlevels(struct fileblock* fb, unsigned short* rlvl, char* runlevels);

extern int mmapfile(struct fileblock* fb, int maxlen);
extern int munmapfile(struct fileblock* fb);
extern int nextline(struct fileblock* f);

static int parseinitline(struct fileblock* fb, int strict);

/* Top-level inittab format */
/* Strict means bail out on errors immediately; with strict=0, it should continue
   as far as possible, assuming it's initial configuration with no fallback. */
int readinittab(const char* file, int strict)
{
	int ret = -1;
	struct fileblock fb = {
		.name = file,
		.line = 0
	};

	if(mmapfile(&fb, -MAXFILE))
		return -1;

	while(nextline(&fb))
		if((ret = parseinitline(&fb, strict)) && strict)
			break;

	munmapfile(&fb);

	return strict ? ret : 0;
}

/* Like strsep(), but using /\s+/ for delimiter */
static char* strssep(char** str)
{
	char* ret = *str;
	char* ptr = ret;

	for(ptr = ret; *ptr; ptr++)
		if(*ptr == ' ' || *ptr == '\t')
			break;
	while(*ptr == ' ' || *ptr == '\t')
		*(ptr++) = '\0';

	*str = ptr;
	return ret;
}

/* Parse current line from fb, the one marked by fb->ls and fb->le. */
static int parseinitline(struct fileblock* fb, int strict)
{
	char* p;
	char* l = fb->ls;

	if(!*l || *l == '#')
		/* empty or comment line */
		return 0;

	if(!(p = strpbrk(l, "= \t:")))
		goto bad;
	else if(*p == ':')
		retwarn(-1, "%s:%i: SysV-style inittab detected, aborting");
	else if(*p == '=')
		return addenviron(l);

	char* code = strssep(&l);
	char* name = strssep(&l);
	/* l is the command here */

	if(code && name && *l)
		return addinitrec(fb, code, name, l, 0);

bad:	retwarn(-1, "%s:%i: bad line", fb->name, fb->line);
}
