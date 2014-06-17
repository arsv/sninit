#define _GNU_SOURCE
#define _BSD_SOURCE
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#include "config.h"
#include "init.h"
#include "init_conf.h"
#include "sys.h"

/* How reconfiguring works:
	1. new config is compiled into newblock
	2. processes not in the new inittab are stopped
	3. remaining pids are transferred from inittab to newblock
	4. newblock replaces inittab
   Note that up until step 4 init uses the old inittab structure, because
   step 2 implies waiting for pids that have no place for them in newtab.

   This file only handles step 1.
   configure() is the entry point. It sets up newblock memory block and
   returns 0 on success.

   In case init gets another reconfigure request while in the main
   loop during step 2, compiled newtab is discarded and we're back to step 1. */

/* During configuration, two additional memblocks are mmaped: newblock and scratchblock.
   newblock becomes cfgblock once configuration is done.
   scratchblock is used to "scratch" arrays of unknown size,
   to put the into newblock later. */

/* Note that until rewirepointers() call late in the process, all pointers
   in struct config, struct initrec and envp aren't real pointers,
   they are offsets from the start of newblock.
   This is to avoid a lot of hassle in case mremap changes the block address. */

extern int state;
extern int currlevel;
extern struct config* cfg;
extern const char* inittab;

struct memblock cfgblock = { NULL };
struct memblock newblock = { NULL };
struct memblock scratchblock = { NULL };

/* top-level functions handling configuration */
int readinittab(const char* file, int strict);		/* /etc/inittab */

static void initcfgblocks(void);	/* set initial values for struct config */
static int finishenvp(void);		/* copy the contents of newenviron to newblock */
static void rewirepointers(void);	/* turn offsets into actual pointers in newblock,
					   assuming it won't be mremapped anymore */
static void transferpids(void);
extern struct initrec* findentry(const char* name);

extern int mmapblock(struct memblock* m, int size);
extern void munmapblock(struct memblock* m);
extern int addptrsarray(offset listoff);


int configure(int strict)
{
	if(mmapblock(&newblock, IRALLOC + sizeof(struct config)))
		goto unmap;
	if(mmapblock(&scratchblock, IRALLOC + sizeof(struct scratch)))
		goto unmap;

	initcfgblocks();

	if(readinittab(inittab, strict))
		/* readinittab does warn() about the reasons, so no need to do it here */
		goto unmap;

	if(finishenvp())
		goto unmap;

	rewirepointers();
	munmapblock(&scratchblock);

	return 0;

unmap:	munmapblock(&newblock);
	munmapblock(&scratchblock);
	return -1;
}

/* newconfig contains complied config block. Set it as mainconfig,
   freeing the old one if necessary.
   PID data transfer also occurs here; the reason is that it must
   be done as close to exchanging pointers as possible, to avoid
   losing dead children in process */
void setnewconf(void)
{
	transferpids();
	munmapblock(&cfgblock);		/* munmapblock can handle empty blocks */

	cfgblock = newblock;
	cfg = (struct config*) cfgblock.addr;
	state &= ~S_RECONFIG;
	newblock.addr = NULL;
}

static void initcfgblocks(void)
{
	struct config* cfg = blockptr(&newblock, 0, struct config*);

	/* newblock has enough space for struct config, see configure() */
	newblock.ptr += sizeof(struct config);
	memset(newblock.addr, 0, sizeof(struct config));

	scratchblock.ptr += sizeof(struct scratch);
	memset(scratchblock.addr, 0, sizeof(struct scratch));

	cfg->inittab = NULL;
	cfg->env = NULL;

	cfg->time_to_restart = 1;
	cfg->time_to_SIGKILL = 2;
	cfg->time_to_skip = 5;
	cfg->slippery = SLIPPERY;

	cfg->logdir = NULL;
}

static int finishenvp(void)
{
	offset off;

	if((off = addptrsarray(TABLIST)) < 0)
		return -1;
	else
		NCF->inittab = NULL + off;

	if((off = addptrsarray(ENVLIST)) < 0)
		return -1;
	else
		NCF->env = NULL + off;

	NCF->initnum = SCR->inittab.count;
	
	return 0;
}

/* parseinittab() fills all pointers in initrecs with offsets from newblock
   to allow using MREMAP_MAYMOVE. Once newblock and newenviron are all
   set up, we need to make those offsets into real pointers */
static inline void* repoint(void* p)
{
	if(p - NULL > newblock.ptr)
		return NULL;	// XXX: should never happen
	return p ? (newblock.addr + (p - NULL)) : p;
}

#define REPOINT(a) a = repoint(a)

/* Warning: while NCF->inittab, NCF->env and initrec.argv-s within inittab
   are arrays of pointers, the fields of NCF are pointers themselves but initrec.argv is not.

   Thus, the contents of all three must be repointed (that's rewireptrsarray)
   but initrec.argv must not be touched, unlike NCF->inittab and NCF->env.

   Since char** or initrec** are not cast silently to void**, there are explicit casts here
   which may mask compiler warnings. */

static void rewireptrsarray(void** a)
{
	void** p;

	for(p = a; *p; p++)
		REPOINT(*p);
}

/* Run repoint() on all relevant pointers within newblock */
static void rewirepointers()
{
	struct initrec** p;

	REPOINT(NCF->inittab);
	rewireptrsarray((void**) NCF->inittab);

	REPOINT(NCF->env);
	rewireptrsarray((void**) NCF->env);

	for(p = NCF->inittab; *p; p++)
		rewireptrsarray((void**) (*p)->argv);
}

/* move child state info from cfgblock to newblock */
/* old inittab is cfg->inittab (which may or may not be CFG->inittab) */
/* new inittab is NCF->inittab */
static void transferpids(void)
{
	struct initrec* p;
	struct initrec* q;
	struct initrec** qq;

	/* initial configuration with no static backup inittab */
	if(!cfg) return;

	for(qq = NCF->inittab; (q = *qq); qq++) {
		/* Prevent w-type entries from being spawned during
		   the next initpass() just because they are new */
		/* This requires (currlevel == nextlevel) which is enforced with S_RECONF. */
		if((q->flags & C_WAIT) && (q->rlvl & (1 << currlevel)))
			q->pid = -1;

		if(!q->name)
			/* can't transfer unnamed entries */
			continue;

		if(!(p = findentry(q->name)))
			/* the entry is genuinely new, nothing to transfer here */
			continue;

		q->pid = p->pid;
		q->flags |= (p->flags & (P_DISABLED | P_SIGTERM | P_SIGKILL | P_ZOMBIE));
		q->lastrun = p->lastrun;
		q->lastsig = p->lastsig;
	}
}
