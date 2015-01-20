#define _GNU_SOURCE
#include "init.h"

/* bits for waitfor */
#define DYING		(1<<0)
#define RUNNING		(1<<1)

extern int state;
extern int currlevel;
extern int nextlevel;
extern int timetowait;
extern struct config* cfg;

extern void spawn(struct initrec* p);
extern void stop(struct initrec* p);

static inline void swapi(int* a, int* b);
static inline int shouldberunning(struct initrec* p);

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
   things get a bit tricky with w-/o-type entries which are traversed
   several times but should only be run once.
   Note to have pid reset to 0, and thus allow re-run, at least one pass
   must be completed with !shouldberunning(p) for the entry. */

#define wtype(p) ((p->flags & (C_ONCE | C_WAIT)) == (C_ONCE | C_WAIT))
#define htype(p) ((p->flags & (C_ONCE | C_WAIT)) == (C_ONCE))
#define owtype(p)  ((p->flags & C_ONCE))
#define hstype(p) (!(p->flags & C_ONCE))
#define slippery(rlvl) (cfg->slippery & rlvl)

void initpass(void)
{
	int waitfor = 0;
	struct initrec** pp;
	struct initrec* p;

	if(!cfg->inittab || cfg->initnum <= 0)
		return;	/* should never happen, but who knows */

	struct initrec** inittab = cfg->inittab;
	struct initrec** initend = cfg->inittab + cfg->initnum - 1;

	state |= S_WAITING;

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
				else if(owtype(p))
					waitfor |= RUNNING;
				continue;
			}

			if(p->pid < 0 && owtype(p))
				continue; /* has been run already */
			if(waitfor && wtype(p))
				return;

			if(hstype(p) && slippery(nextlevel))
				continue; /* these will be killed anyway */

			spawn(p);

			if(owtype(p))
				waitfor |= RUNNING;	/* we're not in nextlevel yet */
			if(wtype(p))
				return;			/* off to wait for this process */
		}

	if(!waitfor)
		state &= ~S_WAITING;
 	if(waitfor || currlevel == nextlevel)
		return;

	/* Nothing more to run, we've done switching runlvls */

	/* One we're here, reset pid for o-type entries, to run them when
	   entering another runlevel with shouldberunning(p) true. */
	for(pp = inittab; (p = *pp); pp++)
		if(!shouldberunning(p) && owtype(p) && (p->pid < 0))
			p->pid = 0;

	if(slippery(nextlevel) && !slippery(currlevel)) {
		/* nextlevel is slippery, turn back to currlevel */
		swapi(&currlevel, &nextlevel);
		/* We've got to make sure pollfds will return immediately. */
		timetowait = 0;
	} else {
		currlevel = nextlevel;
	} if(currlevel == (1<<0)) {
		/* Runlevel 0 (that's 1<<0 = 1) is "slippery" in its own particular way:
		   once it's reached, we've got to kill all that's left and exit
		   the main loop. Since shouldberunning always fails against all-0-bits,
		   this is a simple way to ensure no processes are left behind. */
		nextlevel = 0;
		timetowait = 0;
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

static inline int shouldberunning(struct initrec* p)
{
	if(p->flags & P_DISABLE)
		/* manually disabled */
		return 0;
	if(!(p->rlvl & nextlevel & PRIMASK))
		/* not in this primary level */
		return 0;
	if(!(p->rlvl & SUBMASK))
		/* sublevels not defined = run in all sublevels */
		return 1;
	if(!(p->rlvl & nextlevel & SUBMASK))
		return 0;

	return 1;
}

static inline void swapi(int* a, int* b)
{
	int t = *b; *b = *a; *a = t;
}
