/* Laying out split-but-not-completely-parsed-yet initrecs in newblock */

#include <string.h>
#include "init.h"
#include "init_conf.h"

extern struct memblock newblock;

int prepargv(char* str, char** end);
int setrunlevels(struct initrec* entry, char* runlevels, struct fileblock* fb);
int setflags(struct initrec* entry, char* flagstring, struct fileblock* fb);

extern int addstringarray(struct memblock* m, int n, const char* str, const char* end);
extern int addstrargarray(struct memblock* m, ...);
extern int mextendblock(struct memblock* m, int size, int blocksize);
void linkinitrec(offset entryoff);
void dropinitrec(offset entryoff);

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

	if(mextendblock(&newblock, sizeof(struct initrec), IRALLOC))	
		return -1;

	/* The base structure */
	entryoff = newblock.ptr;
	linkinitrec(entryoff);

	entry = initrecat(&newblock, entryoff); 

	entry->next = NULL;

	memset(entry->name, 0, NAMELEN);
	strncpy(entry->name, name, NAMELEN - 1);

	entry->flags = 0;
	entry->rlvl = 0;

	entry->pid = 0;
	entry->lastrun = 0;
	entry->lastsig = 0;

	/* argv[] must be appended right after struct initrec */
	newblock.ptr += sizeof(struct initrec);

	if(exe) {
		ret = addstrargarray(&newblock, cmd, NULL);
	} else if(cmd[0] == '!') {
		ret = addstrargarray(&newblock, "/bin/sh", "-c", cmd + 1, NULL);
	} else {
		char* arge; int argc = prepargv(cmd, &arge);
		ret = addstringarray(&newblock, argc, cmd, arge);
	} if(ret)
		goto out;

	/* add*array() calls above could very well change newblock.addr, invalidating existing entry value */
	entry = initrecat(&newblock, entryoff); 
	if(setrunlevels(entry, runlvl, fb))
		goto out;
	if(setflags(entry, flags, fb))
		goto out;

	return 0;

out:	dropinitrec(entryoff);
	newblock.ptr = entryoff;
	return -1;
}

/* Update initrec list pointers, including the entry at entryoff in the list */
void linkinitrec(offset entryoff)
{
	SCR->oldend = SCR->newend;

	if(SCR->newend) {
		/* newend >= 0 means this is not the first entry, and newend contains
		   a valid offset of an allocated structure */
		initrecat(&newblock, SCR->newend)->next = NULL + entryoff;
	} else if(!NCF->inittab)
		NCF->inittab = NULL + entryoff;

	SCR->newend = entryoff;
}

/* In case new initrec was not accepted (due to errors etc), it is removed from
   newblock. Because data is added to newblock sequentially, it is enough to
   revert newblock.ptr, and fix whatever changes were done by linkinitrec(). */
void dropinitrec(offset entryoff)
{
	if(!SCR->oldend)
		NCF->inittab = NULL;
	SCR->newend = SCR->oldend;
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

   See doc/sublevels.txt for why sublevels are treated like this. */ 

int setrunlevels(struct initrec* entry, char* runlevels, struct fileblock* fb)
{
	char* p;
	char neg = (*runlevels == '~' ? *(runlevels++) : 0);

	entry->rlvl = 0;

	for(p = runlevels; *p; p++)
		if(*p >= '0' && *p <= '9')
			entry->rlvl |= (1 << (*p - '0'));
		else if(*p >= 'a' && *p <= 'f')
			entry->rlvl |= (1 << (*p - 'a' + 0xa));

	if(!(entry->rlvl & PRIMASK))
		entry->rlvl |= (PRIMASK & ~1);

	if(neg)
		/* Due to the way sublevels are handled, negating them makes no sense */
		entry->rlvl = (~(entry->rlvl & PRIMASK) & PRIMASK) | (entry->rlvl & SUBMASK);

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
	/* exec-side flags */
	{ "abort",	C_USEABRT },
	{ "null",	C_NULL },
	{ "log",	C_LOG },
	{ "tty",	C_TTY },
	/* terminator */
	{ NULL }
};

int setflags(struct initrec* entry, char* flagstring, struct fileblock* fb)
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
void strpull(char* p) { for(;*p;p++) *p = *(p+1); }
/* Input:  |/sbin/foo -a 30 -b "foo bar" -c some\ thing₀| */
/* Output: |/sbin/foo₀-a₀30₀-b₀foo bar₀-c₀some thing₀₀₀₀| */
/* Return: 7 */
int prepargv(char* str, char** end)
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

/* Add a string to one of scratcblock lists (envp and initdir) */
int scratchstring(char listcode, const char* string)
{
	struct stringlist* list = NULL;
	struct stringnode* node;
	int slen = strlen(string);
	int size = sizeof(struct stringnode) + slen + 1;

	if(mextendblock(&newblock, size, IRALLOC))
		return -1;

	/* mextendblock above MAY move scratch.addr! */
	switch(listcode) {
		case 'E': list = &SCR->env; break;
		case 'D': list = &SCR->dir; break;
		default: return -1;
	}

	node = (struct stringnode*)(newblock.addr + newblock.ptr);
	strncpy(node->str, string, slen);

	if(list->last) {
		node = (struct stringnode*)(newblock.addr + list->last);
		node->next = newblock.ptr;
	} else {
		list->head = newblock.ptr;
	};
	list->last = newblock.ptr;

	newblock.ptr += size;

	return list->count++;
}
