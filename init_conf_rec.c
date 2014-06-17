/* Laying out split-but-not-completely-parsed-yet initrecs in newblock */

#include <string.h>
#include <stddef.h>
#include "init.h"
#include "init_conf.h"

extern struct memblock newblock;

global int addinitrec(struct fileblock* fb, char* name, char* runlvl, char* flags, char* cmd, int exe);
global int addenviron(const char* def);

static int prepargv(char* str, char** end);
global int setrunlevels(struct fileblock* fb, unsigned short* rlvl, char* runlevels);
static int setflags(struct fileblock* fb, struct initrec* entry, char* flagstring);

extern int mextendblock(struct memblock* m, int size);
extern int addstruct(int size);
extern int addstring(const char* string);
extern int addstringarray(int n, const char* str, const char* end);
extern int addstrargarray(const char* args[]);

extern int scratchptr(offset listptr, offset ptr);
extern int checkdupname(const char* name);

/* Arguments:
	   name="httpd", runlvl="2345", flags="log,null"
   For argv, there are three options:
	(1) exe=0 argv="/sbin/httpd -f /etc/httpd.conf"
	(2) exe=0 argv="!httpd -f /etc/httpd.conf"
	(3) exe=1 argv="/etc/rc/script"
   Option (1) is parsed in-place, (2) is passed to sh -c, while (3) assumes
   the file itself is executable and no arguments should be passed.
   This all affects only the way initrec->argv is built.  */

/* fb is the block we're parsing currently, used solely for error reporting */

int addinitrec(struct fileblock* fb, char* name, char* runlvl, char* flags, char* cmd, int exe)
{
	offset entryoff;
	struct initrec* entry;
	int ret;

	/* This can (and should) be done early, since it's easier to do when the new
	   initrec is not yet linked to the list. */
	if(checkdupname(name))
		retwarn(-1, "%s:%i: duplicate name %s", fb->name, fb->line, name);

	int entrysize = sizeof(struct initrec);
	if(mextendblock(&newblock, entrysize))	
		return -1;
	entryoff = newblock.ptr;
	entry = blockptr(&newblock, entryoff, struct initrec*); 

	memset(entry->name, 0, NAMELEN);
	strncpy(entry->name, name, NAMELEN - 1);

	entry->flags = 0;
	entry->rlvl = 0;

	entry->pid = 0;
	entry->lastrun = 0;
	entry->lastsig = 0;

	/* Note: argv[] must be appended right after struct initrec */
	newblock.ptr += entrysize;

	if(exe) {
		const char* argv[] = { cmd, NULL };
		ret = addstrargarray(argv);
	} else if(*cmd == '!') {
		for(cmd++; *cmd && *cmd == ' '; cmd++);
		const char* argv[] = { "/bin/sh", "-c", cmd, NULL };
		ret = addstrargarray(argv);
	} else {
		char* arge; int argc = prepargv(cmd, &arge);
		ret = addstringarray(argc, cmd, arge);
	} if(ret)
		goto out;

	/* add*array() calls above could very well change newblock.addr, invalidating existing entry value */
	entry = blockptr(&newblock, entryoff, struct initrec*); 
	if(setrunlevels(fb, &(entry->rlvl), runlvl))
		goto out;
	if(setflags(fb, entry, flags))
		goto out;

	/* initrec has been added successfully, so note its offset to use when building inittab[] later */
	if(scratchptr(TABLIST, entryoff))
		goto out;

	return 0;

out:	/* cancel the entry, resetting newblock.ptr */
	newblock.ptr = entryoff;
	return -1;
}

int addenviron(const char* def)
{
	offset p;
       
	if((p = addstring(def)) < 0)
		return -1;
	if(scratchptr(ENVLIST, p))
		return -1;

	return 0;
}

/* Initialize entry runlevels mask using :runlevels: field from inittab */

/* Cases to consider:
   	(empty)		all runlevels except 0, regardless of sublevels
	123		runlevels 123, regardless of sublevels
	123ab		runlevels 123, but only if sublevels a or b are active
	ab		all runlevels except 0 if sublevels a or b are active
	01		runlevels 0 and 1
   Leaving out primary levels means (PRIMASK & ~1).
   Leaving out sublevels, however, means 0 over SUBMASK.
   This is because (1 << runlevel) is never zero, i.e. there's always one active
   primary level, but zero mask for sublevels is quite possible.

   That's all assuming fb->rlvl = (PRIMASK & ~1), which it is for all inittab
   entries. For initdir entries, fb->rlvl may have different value, in which case
   leaving out pri or sub part means using resp. pri or sub part of fb->rlvl.

   See doc/sublevels.txt for considerations re. sublevels handling.
 
   Note this function is not static, and aside from initrecs it is also used
   for dir-default runlevels in parsedirline(). */ 

int setrunlevels(struct fileblock* fb, unsigned short* rlvl, char* runlevels)
{
	char* p;
	char neg = (*runlevels == '~' ? *(runlevels++) : 0);

	*rlvl = 0;

	for(p = runlevels; *p; p++)
		if(*p >= '0' && *p <= '9')
			*rlvl |= (1 << (*p - '0'));
		else if(*p >= 'a' && *p <= 'f')
			*rlvl |= (1 << (*p - 'a' + 0xa));

	/* The tricky part, handling default runlevels. */
	if(!(*rlvl & PRIMASK))
		*rlvl |= fb->rlvl & PRIMASK;
	if(!(*rlvl & SUBMASK))
		*rlvl |= fb->rlvl & SUBMASK;

	/* Due to the way sublevels are handled, negating them makes no sense */
	if(neg) *rlvl = (~(*rlvl & PRIMASK) & PRIMASK) | (*rlvl & SUBMASK);

	return 0;
}

static struct flagrec {
	char* name;
	int bits;
} flagtbl[] = {
	/* entry type */
	{ "s",		0 },
	{ "respawn",	0 },
	{ "w",		C_ONCE | C_WAIT },
	{ "wait",	C_ONCE | C_WAIT },
	{ "o",		C_ONCE },
	{ "once",	C_ONCE },
	{ "last",	C_LAST },
	{ "l",		C_LAST },
	/* exec-side flags */
	{ "abort",	C_USEABRT },
	{ "null",	C_NULL },
	{ "log",	C_LOG },
	{ "tty",	C_TTY },
	/* terminator */
	{ NULL }
};

static int setflags(struct fileblock* fb, struct initrec* entry, char* flagstring)
{
	char* p;
	struct flagrec* f;

	if(!flagstring || !*flagstring)
		return 0;

	while((p = strsep(&flagstring, ",")) != NULL) {
		for(f = flagtbl; f->name; f++)
			if(!strcmp(p, f->name)) {
				entry->flags |= f->bits;
				break;
			}
		if(!f->name)
			retwarn(-1, "%s:%i: unknown flag \"%s\"", fb->name, fb->line, p);
	}

	return 0;
}

/* Parse the line, counting the entries and simultaineously
   marking them by placing \0 in appropriate places. */
/* Note: str is always 0-terminated, see parseinitline */
/* Note: this changes *str */
static void strpull(char* p) { for(;*p;p++) *p = *(p+1); }
/* Input:  |/sbin/foo -a 30 -b "foo bar" -c some\ thing₀| */
/* Output: |/sbin/foo₀-a₀30₀-b₀foo bar₀-c₀some thing₀₀₀₀| */
/* Return: 7 */
static int prepargv(char* str, char** end)
{
	char* p;
	int argc = 0;
	int state = 1;	/* 4 = after backslash, 2 = in quotes, 1 = in separator / skipping spaces */

	for(p = str; *p; p++) {
		if(state & 4) {
			switch(*p) {
				case 'n': *p = '\n'; break;
				case 't': *p = '\t'; break;
			}
			state &= ~4;
		} else if(state & 2) {
			/* ..but come and think of it, ""s are unnecessary luxury
			   in inittab. Probably got to drop them and leave spaces
			   only, with an added bonus of keeping string length constant */
			if(*p == '"') {
				strpull(p--);
				state &= ~2;
			};
		} else {
			if(state & 1) switch(*p) {
				case '\n':
				case '\t':
				case ' ': strpull(p--); continue;
				default: state &= ~1; argc++;
			} switch(*p) {
				case '\\': state |= 4; strpull(p--); break;
				case '"':  state |= 2; strpull(p--); break;
				case '\n':
				case ' ':
				case '\t': *p = '\0'; state |= 1; break;
			};
		}
	} if(end) *end = p;

	return argc;
}
