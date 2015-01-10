/* Laying out split-but-not-completely-parsed-yet initrecs in newblock */

#include <string.h>
#include <stddef.h>
#include "init.h"
#include "init_conf.h"

extern struct memblock newblock;

global int addenviron(const char* def);
global int addinitrec(struct fileblock* fb, char* code, char* name, char* cmd, int exe);

static int prepargv(char* str, char** end);
static int addrecargv(char* cmd, int exe);
static int setrunflags(struct fileblock* fb, struct initrec* entry, char* flagstring);

extern int addstruct(int size, int extra);
extern int addstring(const char* string);
extern int addstringarray(int n, const char* str, const char* end);
extern int addstrargarray(const char* args[]);

static int linknode(offset listptr, offset nodeptr);
extern int checkdupname(const char* name);

/* Arguments:
	   code="S234", name="httpd"
   For argv, there are three options:
	(1) exe=0 argv="/sbin/httpd -f /etc/httpd.conf"
	(2) exe=0 argv="!httpd -f /etc/httpd.conf"
	(3) exe=1 argv="/etc/rc/script"
   Option (1) is parsed in-place, (2) is passed to sh -c, while (3) assumes
   the file itself is executable and no arguments should be passed.
   This all affects only the way initrec->argv is built.  */

/* fb is the block we're parsing currently, used solely for error reporting */

int addinitrec(struct fileblock* fb, char* code, char* name, char* cmd, int exe)
{
	offset nodeoff;
	offset entryoff;
	struct initrec* entry;
	int ret;

	/* This can (and should) be done early, since it's easier to do when the new
	   initrec is not yet linked to the list. */
	if(name[0] == '-' && !name[1])
		name[0] = '\0';
	else if(checkdupname(name))
		retwarn(-1, "%s:%i: duplicate name %s", fb->name, fb->line, name);

	/* Put ptrnode and struct initrec itself */
	if((nodeoff = addstruct(sizeof(struct ptrnode) + sizeof(struct initrec), 0)) < 0)
		return -1;
	entryoff = nodeoff + sizeof(struct ptrnode);

	/* Put argv[] right after struct initrec */
	if((ret = addrecargv(cmd, exe)))
		goto out;

	/* Initialize the entry. Now because addrecargv() calls above could
	   very well change newblock.addr, the entry pointer must be evaluated here
	   and not next to entryoff above */
	entry = newblockptr(entryoff, struct initrec*); 

	memset(entry->name, 0, NAMELEN);
	strncpy(entry->name, name, NAMELEN - 1);

	entry->pid = 0;
	entry->lastrun = 0;
	entry->lastsig = 0;

	if(setrunflags(fb, entry, code))
		goto out;

	/* initrec has been added successfully, so note its offset to use when
	   building inittab[] later */
	linknode(TABLIST, nodeoff);

	return 0;

out:	/* cancel the entry, resetting newblock.ptr */
	newblock.ptr = nodeoff;
	return -1;
}

static int addrecargv(char* cmd, int exe)
{
	if(exe) {
		const char* argv[] = { cmd, NULL };
		return addstrargarray(argv);
	} else if(*cmd == '!') {
		for(cmd++; *cmd && *cmd == ' '; cmd++);
		const char* argv[] = { "/bin/sh", "-c", cmd, NULL };
		return addstrargarray(argv);
	} else {
		char* arge; int argc = prepargv(cmd, &arge);
		return addstringarray(argc, cmd, arge);
	}
}

int addenviron(const char* def)
{
	int len = strlen(def);
	offset nodeoff;

	if((nodeoff = addstruct(sizeof(struct ptrnode) + len + 1, 0)) < 0)
		return -1;

	offset dstoff = nodeoff + sizeof(struct ptrnode);
	char* dst = newblockptr(dstoff, char*);

	strncpy(dst, def, len + 1);
	linknode(ENVLIST, nodeoff);

	return 0;
}

static int linknode(offset listptr, offset nodeptr)
{
	struct ptrnode* node = newblockptr(nodeptr, struct ptrnode*);
	struct ptrlist* list = newblockptr(listptr, struct ptrlist*);

	if(!list->head)
		list->head = nodeptr;
	if(list->last)
		newblockptr(list->last, struct ptrnode*)->next = nodeptr;

	node->next = 0;

	list->last = nodeptr;
	list->count++;

	return 0;
}

static struct flagrec {
	char c;
	int bits;
} flagtbl[] = {
	/* entry type */
	{ 'S',	0 },
	{ 'W',	C_ONCE | C_WAIT },
	{ 'O',	C_ONCE },
	{ 'H',	C_WAIT },
	/* process control flags */
	{ 'A',	C_USEABRT },
	/* exec-side flags */
	{ 'N',	C_NULL },
	{ 'Y',	C_TTY },
	/* terminator */
	{  0  }
};

/* Parse runlevels and flags (1st initline field) into entry->rlvl
   and entry->flags. Typical input: code="S123N".

   When specified, runlevels are translated as is (i.e. "12a" = R1 | R2 | Ra),
   however there are some special cases:
	(none)		same as "123456789" (not 0 is *not* in the mask)
	~12		all but 12, that is, "03456789"
	~12a		same as "03456789a;
			pri levels are inverted, sublevels aren't

   See doc/sublevels.txt for considerations re. sublevels handling. */

static int setrunflags(struct fileblock* fb, struct initrec* entry, char* code)
{
	struct flagrec* f;
	char* p;
	int rlvl = 0;
	int neg = 0;
	int flags = 0;

	for(p = code; *p; p++) {
		if(*p == '-')
			continue;
		else if(*p == '~')
			neg = 1;
		else if(*p >= '0' && *p <= '9')
			rlvl |= (1 << (*p - '0'));
		else if(*p >= 'a' && *p <= 'f')
			rlvl |= (1 << (*p - 'a' + 10));
		else {
			for(f = flagtbl; f->c && f->c != *p; f++)
				;
			if(f->c)
				flags |= f->bits;
			else
				retwarn(-1, "%s:%i: unknown flag %c", fb->name, fb->line, *p);
		}
	}

	if(!(rlvl & PRIMASK))
		rlvl |= (PRIMASK & ~1);

	/* Due to the way sublevels are handled, simply negating them makes no sense */
	if(neg) rlvl = (~(rlvl & PRIMASK) & PRIMASK) | (rlvl & SUBMASK);

	entry->rlvl = rlvl;
	entry->flags = flags;

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
