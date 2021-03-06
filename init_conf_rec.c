#include <string.h>
#include <stddef.h>
#include "init.h"
#include "init_conf.h"

/* Both initrecs and environment lines are initially placed in their
   respective linked lists (struct scratch), with the list nodes scattered
   between initrecs. The lists are only used to create inittab[] and env[]
   in struct config, but they are left in place anyway, since recovering
   the space is more trouble than it's worth.

   Why not use the lists directly?
   Well execve(2) takes envp[], and initpass iterates over initrecs in both
   directions, which turns out to be easier with inittab[] vs some kind
   of doubly-linked list. */

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

/* It's an error to have two entries with the same (non-empty) name
   as it makes telinit commands ambiguous, so dupes are checked and
   reported at parsing stage.

   This is called during initrec parsing, way before NCF->inittab array
   is formed. So it can't use NCF->inittab. Instead, it should use
   SCR->inittab (the offset list) to find location of entries added so far. */

static int checkdupname(const char* name)
{
	offset po = SCR->inittab.head;
	struct ptrnode* n;
	struct initrec* p;

	while(po) {
		n = newblockptr(po, struct ptrnode*);
		p = newblockptr(po + sizeof(struct ptrnode), struct initrec*);

		if(p->name[0] && !strcmp(p->name, name))
			return -1;

		po = n->next;
	}

	return 0;
}

/* addinitrec() and addenviron() are called for each parsed inittab line
   and their task is to copy stuff from the fileblock being parsed
   over to newblock.

   [fb: /etc/inittab]
   	name="tty" rlvl="fast" cmd=[/sbin/getty, 115200, /dev/ttyS0] exe=0

   [fb: non-executable /etc/rc/httpd]
   	name="httpd" rlvl="" cmd=[/sbin/httpd] exe=0

   [fb: executable /etc/rc/squid]
	name="squid" rlvl=":a" cmd=[/etc/rc/squid] exe=1

   Non-zero exe means cmd is the name of the script to run. See addrecargv(). */

int addinitrec(const char* name, const char* rlvl, char* cmd, int exe)
{
	offset nodeoff;
	offset entryoff;
	struct initrec* entry;
	int ret;

	/* This can (and should) be done early, since it's easier to do
	   when the new initrec is not yet linked to the list. */
	if(name[0] == '-' && !name[1])
		name = "";
	else if(checkdupname(name))
		retwarn(-1, "%s:%i: duplicate name %s", FBN, FBL, name);

	/* Put ptrnode and struct initrec itself */
	const int nodesize = sizeof(struct ptrnode) + sizeof(struct initrec);
	if((nodeoff = extendblock(nodesize)) < 0)
		return -1;
	entryoff = nodeoff + sizeof(struct ptrnode);

	entry = newblockptr(entryoff, struct initrec*);

	memset(entry->name, 0, NAMELEN);
	strncpy(entry->name, name, NAMELEN - 1);

	entry->pid = 0;
	entry->lastrun = 0;
	entry->lastsig = 0;

	/* This should *NOT* move newblock.ptr to allow proper argv layout. */
	if(setrunflags(entry, rlvl))
		goto out;

	/* Put argv[] right after struct initrec. */
	if((ret = addrecargv(entry, cmd, exe)))
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
	const int nodesize = sizeof(struct ptrnode) + len + 1;

	if((nodeoff = extendblock(nodesize)) < 0)
		return -1;

	offset dstoff = nodeoff + sizeof(struct ptrnode);
	char* dst = newblockptr(dstoff, char*);

	strncpy(dst, def, len + 1);
	linknode(ENVLIST, nodeoff);

	return 0;
}

