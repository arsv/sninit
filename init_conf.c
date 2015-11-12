#include <string.h>
#include <stddef.h>
#include "config.h"
#include "init.h"
#include "init_conf.h"
#include "scope.h"
#include "sys.h"

/* How reconfiguring works:

   	0. newblock is mmapped
	1. new config is compiled into newblock
	2. processes not in the new inittab are stopped
	3. remaining pids are transferred from inittab to newblock
	4. newblock replaces inittab, cfgblock is munmapped

   Up until step 4 init uses the old inittab structure, because step 2
   implies waiting for pids that have no place for them in newtab.

   This file only handles step 1. configure() is the entry point.
   It sets up newblock and returns 0 on success.

   In case init gets another reconfigure request while in the main
   loop during step 2, compiled newtab is discarded and we're back
   to step 1.

   Note that until rewirepointers() call late in the process, all pointers
   in struct config, struct initrec and envp aren't real pointers, they are
   offsets from the start of newblock. This is to avoid a lot of hassle
   in case mremap changes the block address. */

extern int state;
extern int currlevel;
extern struct config* cfg;

extern struct nblock newblock;

/* default/built-in stuff */
const char* inittab = INITTAB;
const char* initdir = INITDIR;

export int configure(int strict);
export void setnewconf(void);

/* top-level functions handling configuration */
extern int readinittab(const char* file, int strict);
extern int readinitdir(const char* dir, int strict);

local int finishinittab(void);
local void rewirepointers(void);

local void transferpids(void);
extern struct initrec* findentry(const char* name);
extern int levelmatch(struct initrec* p, int lmask);

extern int mmapblock(int size);
extern void munmapblock(void);
export void exchangeblocks(void);
extern int addptrsarray(offset listoff, int terminate);
extern offset extendblock(int size);

/* If successful, configure() leaves a valid struct config in newblock.
   Otherwise, it should clean up after itself.

   S_RECONFIG must be set in state to let main() know it's got to
   pick up the new configuration, but that is done outside of configure
   because the two places that call configure() handle failure a bit
   differently.

   configure() itself does not replace cfg.
   That is done later by setnewconf(), called later from main(). */

int configure(int strict)
{
	const int headersize = sizeof(struct config) + sizeof(struct scratch);

	if(mmapblock(headersize))
		goto nomap;

	if(readinittab(inittab, strict))
		goto unmap;
#ifdef INITDIR
	if(readinitdir(initdir, strict))
		goto unmap;
#endif

	if(finishinittab())
		goto unmap;

	rewirepointers();

	/* check if there are any entries at all */
	if(NCF->initnum)
		return 0;	/* ok, we're good */

	warn("no entries found in inittab");

unmap:	munmapblock();
nomap:	return -1;
}

/* Once all initrecs are in place, inittab[] and env[] pointer arrays
   are appended, with pointers (well offsets at this point) referring
   back to initrecs. */

int finishinittab(void)
{
	offset off;

	if((off = addptrsarray(TABLIST, NULL_BOTH)) < 0)
		return -1;
	else
		NCF->inittab = NULL + off;

	if((off = addptrsarray(ENVLIST, NULL_BACK)) < 0)
		return -1;
	else
		NCF->env = NULL + off;

	NCF->initnum = SCR->inittab.count;
	
	return 0;
}

/* addinitrec() fills all pointers in initrecs with offsets from newblock
   to allow using MREMAP_MAYMOVE. Once newblock and newenviron are all
   set up, we need to make those offsets into real pointers ("repoint" them)
   by adding the base address of newblock.

   Because the offsets are pointer-typed and (pointer + pointer) operation
   is illegal, we turn them into integers by subtracting NULL. */

void* repoint(void* p)
{
	if(p - NULL > newblock.ptr)
		return NULL;	// XXX: should never happen
	return p ? (newblock.addr + (p - NULL)) : p;
}

#define REPOINT(a) a = repoint(a)

/* Warning: while NCF->inittab, NCF->env and initrec.argv-s within inittab
   are arrays of pointers, the fields of NCF are pointers themselves
   but initrec.argv is not.

   Thus, the contents of all three must be repointed (that's rewireptrsarray)
   but initrec.argv must not be touched, unlike NCF->inittab and NCF->env.

   Since char** or initrec** are not cast silently to void**, there are
   explicit casts here which may mask compiler warnings. */

void rewireptrsarray(void** a)
{
	void** p;

	for(p = a; *p; p++)
		REPOINT(*p);
}

void rewirepointers()
{
	struct initrec** pp;

	REPOINT(NCF->inittab);
	rewireptrsarray((void**) NCF->inittab);

	REPOINT(NCF->env);
	rewireptrsarray((void**) NCF->env);

	for(pp = NCF->inittab; *pp; pp++)
		rewireptrsarray((void**) (*pp)->argv);
}

/* Once initpass reports it's ok to switch configurations,
   main calls setnewconf. By this point, all live pids and process
   flags are still in the (old) cfg, so setnewconf starts by copying
   anything relevant over to newblock.
   After that, the blocks are exchanged and we're done.

   This is the second entry point in this file. */

void setnewconf(void)
{
	transferpids();
	cfg = newblockptr(0, struct config*);
	exchangeblocks();
}

/* Old inittab is cfg->inittab (which may or may not be CFG->inittab),
   new inittab is NCF->inittab. Old inittab may be missing during initial
   configuration without built-in one.

   Still, even without cfg we need to initialize pids of run-once entries. */

void transferpids(void)
{
	struct initrec* p;
	struct initrec* q;
	struct initrec** qq;

	for(qq = NCF->inittab; (q = *qq); qq++) {
		/* Prevent w-type entries from being spawned during
		   the next initpass() just because they are new.
		   This requires (currlevel == nextlevel) which is enforced
		   with S_RECONF. */
		if((q->flags & C_ONCE) && levelmatch(q, currlevel))
			q->pid = -1;

		if(!cfg) /* first call, no inittab to transfer pids from */
			continue;

		if(!*q->name) /* can't transfer unnamed entries */
			continue;

		if(!(p = findentry(q->name)))
			/* the entry is new, nothing to transfer here */
			continue;

		q->pid = p->pid;
		q->flags |= (p->flags & (P_MANUAL | P_FAILED | P_WAS_OK));
		q->lastrun = p->lastrun;
		q->lastsig = p->lastsig;
	}
}

/* Make type* array[] style structure in newblock from a ptrlist
   located at listoff in newblock. The array is NULL-terminated
   at the back and/or at the front.
   (inittab needs front NULL for reverse pass in initpass)

   Because the pointers are only available when all the data has
   been placed, the pointer array ends up after the actual data
   in newblock, with back-referencing pointers.

   This function is used to lay out config.inittab and config.env,
   but not for initrec.argv which gets a different treatment. */

int addptrsarray(offset listoff, int terminate)
{
	struct ptrlist* list = newblockptr(listoff, struct ptrlist*);

	int rem = list->count;
	int ptrn = rem;
	void** ptrs;
	struct ptrnode* node;
	offset nodeoff;		/* in scratchblock */
	offset ptrsoff;		/* in newblock */

	if(rem < 0) return -1;

	if(terminate & NULL_FRONT) ptrn++;
	if(terminate & NULL_BACK ) ptrn++;

	if((ptrsoff = extendblock(ptrn*sizeof(void*))) < 0)
		return -1;

	ptrs = newblockptr(ptrsoff, void**);

	/* leading NULL pointer is used as terminator
	   when traversing inittab backwards */
	if(terminate & NULL_FRONT) {
		*(ptrs++) = NULL;
		ptrsoff += sizeof(void*);
	}

	/* set up offsets; repoiting will happen later */
	for(nodeoff = list->head; rem && nodeoff; rem--) {
		node = newblockptr(nodeoff, struct ptrnode*);
		*(ptrs++) = NULL + nodeoff + sizeof(struct ptrnode);
		nodeoff = node->next;
	} if(rem)
		/* less elements than expected; this may break initpass,
		   so let's not take chances */
		return -1;

	if(terminate & NULL_BACK)
		*ptrs = NULL;

	return ptrsoff;
}
