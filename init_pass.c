#define _GNU_SOURCE
#include "init.h"
#include "config.h"
#include "scope.h"

/* bits for waitfor */
#define DYING		(1<<0)
#define RUNNING		(1<<1)

extern int state;
extern int currlevel;
extern int nextlevel;
extern int timetowait;
extern struct config* cfg;

export void initpass(void);
export int levelmatch(struct initrec* p, int level);

extern void spawn(struct initrec* p);
extern void stop(struct initrec* p);

local void swapi(int* a, int* b);
local int shouldberunning(struct initrec* p);
local void switchtonextlevel(void);

/* Initpass: go through inittab, (re)starting entries
   that need to be (re)started and killing entries that should be killed.

   There are two principal passes over inittab: bottom-to-top that kills
   processes first, and top-to-bottom that spawns processes. Backwards
   pass is needed to C_WAIT without C_ONCE, i.e. waiting for things to
   die before killing certain s-type records (syslog primarily).

   In case a C_WAIT entry is reached during either pass, relevant action
   is performed and initpass return.
   SIGCHLD will arrive on entry completition, triggering another initpass.
   Blocking wait is never used, init waits for w-type entries in ppoll().
 
   Because no explicit list pointer is kept during runlevel switching,
   things get a bit tricky with r-type entries which are traversed
   several times but should only be run once.
   Note to have pid reset to 0, and thus allow re-run, at least one pass
   must be completed with !shouldberunning(p) for the entry. */

#define wtype(p) ((p->flags & (C_ONCE | C_WAIT)) == (C_ONCE | C_WAIT))
#define htype(p) ((p->flags & (C_ONCE | C_WAIT)) == (C_ONCE))
#define ewtype(p)  ((p->flags & C_ONCE))
#define hstype(p) (!(p->flags & C_ONCE))

#define slippery(rlvl) (SLIPPERY & rlvl)

void initpass(void)
{
	int waitfor = 0;
	struct initrec** pp;
	struct initrec* p;

	if(!cfg->inittab || cfg->initnum <= 0)
		goto done; /* should never happen, but who knows */

	struct initrec** inittab = cfg->inittab;
	struct initrec** initend = cfg->inittab + cfg->initnum - 1;

	/* Kill pass, reverse order */
	for(pp = initend; (p = *pp); pp--)
		if(!shouldberunning(p) && p->pid > 0)
		{
			stop(p);

			waitfor |= DYING;

			if(htype(p))
				return;
		}

	/* Run pass, direct order */
	for(pp = inittab; (p = *pp); pp++)
		if(shouldberunning(p))
		{
			if(p->pid > 0) {
				if(wtype(p))
					return;
				else if(ewtype(p))
					waitfor |= RUNNING;
				continue;
			}

			if(p->pid < 0 && ewtype(p))
				continue; /* has been run already */
			if(waitfor && wtype(p))
				return;

			if(hstype(p) && slippery(nextlevel))
				continue; /* these will be killed anyway */

			spawn(p);

			if(ewtype(p))
				waitfor |= RUNNING;	/* we're not in nextlevel yet */
			if(wtype(p))
				return;			/* off to wait for this process */
		}

	if(waitfor)
		return;

done:	if(nextlevel == (1<<0)) {
		/* level 0 is slippery in its own particular way */
		currlevel = nextlevel;
		nextlevel = 0;
		timetowait = 0;
	} else if(currlevel != nextlevel)
		switchtonextlevel();
}

local void switchtonextlevel(void)
{
	struct initrec** inittab = cfg->inittab;
	struct initrec** pp;
	struct initrec* p;

	/* One we're here, reset pid for r-type entries, to run them when
	   entering another runlevel with shouldberunning(p) true. */
	for(pp = inittab; (p = *pp); pp++)
		if(!shouldberunning(p) && ewtype(p) && (p->pid < 0))
			p->pid = 0;

	if(slippery(nextlevel) && !slippery(currlevel)) {
		/* nextlevel is slippery, turn back to currlevel */
		swapi(&currlevel, &nextlevel);
		/* We've got to make sure pollfds will return immediately. */
		timetowait = 0;
	} else {
		currlevel = nextlevel;
	}
}

/* Should we start $p for nextlevel?

        PRIMASK:	      xxxxxxxxxx
	nextlevel:	--dc--------3---
   	p->rlvl:	---c-a----5432--
	SUBMASK:        xxxxxx

   To say yes, we need to make sure there'a a bit for
   current primary runlevel, and that if there are bits
   set for sublevels, at least one of them matches a bit
   in nextlevel.

   Note (p->rlvl & SUBMASK == 0) means "disregard sublevels",
   but (p->rlvl & SUBMASK == SUBMASK) means "run in *any* sublevel" and excludes
   no-active-sublevels case. */

int shouldberunning(struct initrec* p)
{
	if(p->flags & (P_MANUAL | P_FAILED))
		return 0; /* disabled (telinit or respawning too fast) */
	else
		return levelmatch(p, nextlevel);
}

/* transferpids() needs to call this as well, but using currlevel
   instead of nextlevel, so there is shouldberunning() for local
   use and levelmatch there. */

int levelmatch(struct initrec* p, int level)
{
	int go = (p->flags & C_INVERT ? 0 : 1);

	if(!(p->rlvl & level & PRIMASK))
		/* not in this primary level */
		return !go;
	if(!(p->rlvl & SUBMASK))
		/* sublevels not defined = run in all sublevels */
		return go;
	if(!(p->rlvl & level & SUBMASK))
		return !go;

	return go;
}

void swapi(int* a, int* b)
{
	int t = *b; *b = *a; *a = t;
}
