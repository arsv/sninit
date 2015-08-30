#include <string.h>
#include <stddef.h>
#include "init.h"
#include "init_conf.h"
#include "scope.h"

/* addinitrec() and addenviron() are called for each parsed inittab line
   and their task is to copy stuff from the fileblock being parsed
   over to newblock. */

extern struct memblock newblock;

export int addenviron(const char* def);
export int addinitrec(struct fileblock* fb, char* name, char* flags, char* cmd, int exe);

local int prepargv(char* str, char** end);
local int addrecargv(char* cmd, int exe);
extern int setrunflags(struct fileblock* fb, struct initrec* entry, char* flags);

extern int addstruct(int size, int extra);
extern int addstringarray(int n, const char* str, const char* end);
extern int addstrargarray(const char* args[]);

local int linknode(offset listptr, offset nodeptr);
extern int checkdupname(const char* name);

/* Context:

	fileblock=(mmaped /etc/inittab) name="tty" rlvl="fast"
		cmd=[/sbin/getty, 115200, /dev/ttyS0] exe=0
	fileblock=(mmaped /etc/rc/httpd) name="httpd" rlvl=""
		cmd=[/sbin/httpd] exe=0
	fileblock=(mmaped /etc/rc/squid) name="squid" rlvl=":a"
		cmd=[/etc/rc/squid] exe=1

   Non-zero exe means cmd is the name of the script to run, and need
   not be parsed. See addrecargv() below.
*/

int addinitrec(struct fileblock* fb, char* name, char* rlvl, char* cmd, int exe)
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

	if(setrunflags(fb, entry, rlvl))
		goto out;

	/* initrec has been added successfully, so note its offset to use when
	   building inittab[] later */
	linknode(TABLIST, nodeoff);

	return 0;

out:	/* Cancel the entry, resetting newblock.ptr
	   This is enough to completely undo the effect of this function,
	   assuming linknode hasn't been called to change values before
	   the initial newblock.ptr (saved as nodeoff) */
	newblock.ptr = nodeoff;
	return -1;
}

/* The second entry point here, used for environment lines in inittab.
   def is something like "PATH=/bin/sh" somewhere inside fb */

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

/* Lay out argv[] array right after its parent initrec.
   For the command, there are three options:
	(1) exe=0 argv="/sbin/httpd -f /etc/httpd.conf"
	(2) exe=0 argv="!httpd -f /etc/httpd.conf"
	(3) exe=1 argv="/etc/rc/script"
   Option (1) is parsed in-place, (2) is passed to sh -c, while (3) assumes
   the file itself is executable and no arguments should be passed.
   This all affects only the way initrec.argv is built. */

int addrecargv(char* cmd, int exe)
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

int linknode(offset listptr, offset nodeptr)
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

/* Parse the line, counting the entries and simultaineously
   marking them by placing \0 in appropriate places. */
/* Note: str is always 0-terminated, see parseinitline */
/* Note: this changes *str */
local void strpull(char* p) { for(;*p;p++) *p = *(p+1); }
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
