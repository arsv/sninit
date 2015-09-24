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
   It sets up newblock memory block and returns 0 on success.

   In case init gets another reconfigure request while in the main
   loop during step 2, compiled newtab is discarded and we're back to step 1. */

/* Note that until rewirepointers() call late in the process, all pointers
   in struct config, struct initrec and envp aren't real pointers,
   they are offsets from the start of newblock.
   This is to avoid a lot of hassle in case mremap changes the block address. */

extern int state;
extern int currlevel;
extern struct config* cfg;

/* default/built-in stuff */
const char* inittab = INITTAB;
const char* initdir = INITDIR;

struct memblock cfgblock = { NULL };
struct memblock newblock = { NULL };

export int configure(int strict);
export void setnewconf(void);

/* top-level functions handling configuration */
extern int readinittab(const char* file, int strict);
extern int readinitdir(const char* dir, int strict);

local void initcfgblocks(void);		/* set initial values for struct config */
local int finishinittab(void);		/* copy the contents of newenviron to newblock */
local void rewirepointers(void);	/* turn offsets into actual pointers in newblock,
					   assuming it won't be mremapped anymore */
local void transferpids(void);
extern struct initrec* findentry(const char* name);
extern int levelmatch(struct initrec* p, int lmask);

extern int mmapblock(struct memblock* m, int size);
extern void munmapblock(struct memblock* m);
extern int addptrsarray(offset listoff, int terminate);

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
	if(mmapblock(&newblock, IRALLOC + sizeof(struct config) + sizeof(struct scratch)))
		goto unmap;

	initcfgblocks();

	if(readinittab(inittab, strict))
		goto unmap;
#ifdef INITDIR
	if(readinitdir(initdir, strict))
		goto unmap;
#endif

	if(finishinittab())
		goto unmap;

	rewirepointers();

	return 0;

unmap:	munmapblock(&newblock);
	return -1;
}

/* We start by creating the header of the struct in the newly-allocated
   block, then let addinitrec() fill the space with the compiled process
   entries and environment variables.

   Both initrecs and environment lines are initially placed in their
   respective linked lists (struct scratch), with the list nodes scattered
   between initrecs. The lists are only used to create inittab[] and env[]
   in struct config, but they are left in place anyway, since recovering
   the space is more trouble than it's worth.

   Why not use the lists directly?
   Well execve(2) takes envp[], and initpass iterates over initrecs in both
   directions, which turns out to be easier with inittab[] vs some kind
   of doubly-linked list. */

void initcfgblocks(void)
{
	struct config* cfg = newblockptr(0, struct config*);

	/* newblock has enough space for struct config, see configure() */
	int nblen = sizeof(struct config) + sizeof(struct scratch);
	newblock.ptr += nblen;
	memset(newblock.addr, 0, nblen);

	cfg->inittab = NULL;
	cfg->env = NULL;
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
	munmapblock(&cfgblock);	 /* munmapblock can handle empty blocks */

	cfgblock = newblock;
	cfg = (struct config*) cfgblock.addr;
	state &= ~S_RECONFIG;
	newblock.addr = NULL;
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

		if(!q->name) /* can't transfer unnamed entries */
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
