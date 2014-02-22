#define _GNU_SOURCE
#define _BSD_SOURCE
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
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

/* During configuration, one additional memblock, newconfig, is mapped.
   Once configuration is done, it become cfgblock, and the old cfgblock gets unmapped.
   Being a drop-in replacement, newblock starts with struct config,
   but has an additional struct scratch following it which is used to hold
   temporary values needed during configuration. */

/* Note that until rewirepointers() call late in the process, all pointers
   in struct config, struct initrec and envp aren't real pointers,
   they are offsets from the start of newblock.
   This is to avoid a lot of hassle in case mremap changes the block address.  */

extern int state;
extern int currlevel;
extern struct config* cfg;
extern const char* inittab;

struct memblock cfgblock = { NULL };
struct memblock newblock = { NULL };

/* top-level functions handling sinit configuration */
int readinittab(const char* file, int strict);		/* /etc/inittab */

void initcfgblocks(void);	/* set initial values for struct config */
int finishenvp(void);		/* copy the contents of newenviron to newblock */
void rewirepointers(void);	/* turn offsets into actual pointers in newblock,
					   assuming it won't be mremapped anymore */
void transferpids(void);
struct initrec* findentry(const char* name);

extern int mmapblock(struct memblock* m, int size);
extern void munmapblock(struct memblock* m);
extern int addstringptrs(struct memblock* m, struct stringlist* l);

void rewireenvp(char*** envp);


int configure(int strict)
{
	if(mmapblock(&newblock, IRALLOC + sizeof(struct config) + sizeof(struct scratch)))
		goto unmap;

	initcfgblocks();

	if(readinittab(inittab, strict))
		/* readinittab does warn() about the reasons, so no need to do it here */
		goto unmap;

	if(finishenvp())
		goto unmap;

	rewirepointers();

	return 0;

unmap:	munmapblock(&newblock);
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

void initcfgblocks(void)
{
	struct config* cfg = (struct config*) newblock.addr;
	/* newblock has enough space for struct config, see configure() */
	newblock.ptr += sizeof(struct config) + sizeof(struct scratch);
	memset(newblock.addr, 0, sizeof(struct config) + sizeof(struct scratch));

	cfg->inittab = NULL;
	cfg->env = NULL;

	cfg->time_to_restart = 1;
	cfg->time_to_SIGKILL = 2;
	cfg->time_to_skip = 5;
	cfg->slippery = SLIPPERY;

	cfg->logdir = NULL;
}

int finishenvp(void)
{
	int envoff;

	if((envoff = addstringptrs(&newblock, &SCR->env)) < 0)
		return -1;

	NCF->env = NULL + envoff;
	
	return 0;
}

/* parseinittab() fills all pointers in initrecs with offsets from newblock
   to allow using MREMAP_MAYMOVE. Once newblock and newenviron are all
   set up, we need to make those offsets into real pointers */
void* repoint(struct memblock* m, void* p)
{
	return p ? (m->addr + (p - NULL)) : p;
}
#define REPOINT(p) p = repoint(&newblock, p)

/* Run repoint() on all relevant pointers within newblock */
void rewirepointers()
{
	struct initrec* p;
	char** a;

	if(SCR->newend < 0)
		return;

	if(NCF->inittab) {
		REPOINT(NCF->inittab);
		for(p = NCF->inittab; p; p = p->next) {
			REPOINT(p->next);
			for(a = p->argv; *a; a++)
				REPOINT(*a);
			REPOINT(*a); /* terminating NULL pointer */
		}
	}

	rewireenvp(&(NCF->env));
}

void rewireenvp(char*** envp)
{
	char** a;

	if(!*envp)
		return;

	REPOINT(*envp);
	for(a = *envp; *a; a++)
		REPOINT(*a);
}

/* move child state info from cfgblock to newblock */
/* old inittab is CFG->inittab */
/* new inittab is NCF->inittab */
void transferpids(void)
{
	struct initrec* p;
	struct initrec* q;

	/* initial configuration with no static backup inittab */
	if(!cfg) return;

	for(q = NCF->inittab; q; q = q->next) {
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
		q->flags |= (p->flags & (C_DISABLED | P_SIGTERM | P_SIGKILL | P_ZOMBIE));
		q->lastrun = p->lastrun;
		q->lastsig = p->lastsig;
	}
}

/* Note: an entry from the primary inittab, cfg->inittab, the one that initpass uses.
   It may or may not be located in cfgblock, depends on whether it's a static initial
   config or something compiled by configure() */
struct initrec* findentry(const char* name)
{
	struct initrec* p;

	for(p = cfg->inittab; p; p = p->next)
		if(p->name && !strcmp(p->name, name))
			return p;

	return NULL;
}
