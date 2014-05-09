#include "../init.h"
#include "test.h"

extern int state;
extern int currlevel;
extern int nextlevel;
extern int sublevels;

extern char passlog[];
extern void reset(void);
extern void died(struct initrec* p);
extern void killed(struct initrec* p);
extern void initpass(void);

struct initrec I0;
struct initrec I1;
struct initrec I2;
struct initrec I3;
struct initrec I4;
struct initrec I5;
struct initrec I6;

struct config testconfig = { .inittab = &I0 };
struct config* cfg = &testconfig;

#define R1 (1<<1)
#define R2 (1<<2)
#define R3 (1<<3)
#define R12 (R1|R2)
#define R13 (R1|R3)
#define R123 (R1|R2|R3)
#define Ra (1<<0xa)
#define R1a (R1|Ra)
#define SM SUBMASK

/* Startup: I1 * I2 * I3 I4 I5 * rlswitch */
struct initrec I0 = { .next = &I1,  .prev = NULL,.pid = 0, .name = "i0", .rlvl = R1,  .flags = C_ONCE };
struct initrec I1 = { .next = &I2,  .prev = &I0, .pid = 0, .name = "i1", .rlvl = R12, .flags = C_ONCE };
struct initrec I2 = { .next = &I3,  .prev = &I1, .pid = 0, .name = "i2", .rlvl = R12, .flags = C_ONCE | C_WAIT };
struct initrec I3 = { .next = &I4,  .prev = &I2, .pid = 0, .name = "i3", .rlvl = R1,  .flags = C_LAST };
struct initrec I4 = { .next = &I5,  .prev = &I3, .pid = 0, .name = "i4", .rlvl = R12, .flags = C_ONCE };
struct initrec I5 = { .next = &I6,  .prev = &I4, .pid = 0, .name = "i5", .rlvl = R12, .flags = 0 };
struct initrec I6 = { .next = NULL, .prev = &I5, .pid = 0, .name = "i6", .rlvl = R1a, .flags = 0 };

#define Q(t) { reset(); initpass(); S(passlog, t); }
#define Qq(t) Q(t); Q("")
#define D(i) died(&i)
#define K(i) killed(&i)
#define N(r) nextlevel = r

int main(void)
{
	currlevel = 0;
	nextlevel = R1;
	state = 0;

	/* bootup — moving from R0 to level R1 */
	Qq("+i0+i1");		/* start o-entries, do NOT start w-entry until both o-entries are dead */
	D(I1); Q("");		/* i0 is still running, do not proceed to i2 */
	D(I0); Q("+i2");	/* i1 died, ok to start i2 */
	D(I2); Q("+i3+i4+i5");	/* w-type done, ok to start the rest except for I6 which is +a only */
	A(currlevel != R1);	/* should NOT switch until I4 exist */
	Q("");			/* nothing to do yet */
	D(I3); Qq("+i3");	/* it's an s-type entry, got to restrt it */
	D(I4); Q("");		/* do not restart it */
	A(currlevel == R1);	/* and since that was the last ONCE entry, we're in R1 */

	/* Now we're in L1 */
	D(I5); Qq("+i5");	/* s-type, go to restart */

	/* Let's switch to R2. There's almost no difference, only i3 is to be stopped. */
	N(R2);			/* note initpass SHOULD call stop() here second time, */
	Q("-i3"); Q("-i3");	/*   to check for timeouts and possibly force-kill the process */
	A(currlevel != R2);	/* i3 is not yet dead */
	K(I3); Q("");
	A(currlevel == R2);	/* i3 died, ok to change RL now */

	/* Back to R1 */
	N(R1); Qq("+i0+i3");	/* got to re-run i0, and restart i3 */
	A(currlevel == R2);	/* i0 is still running */
	D(I0); Q("");		/* i0 died, do not restart it... */
	A(currlevel == R1);	/* but do change runlevel */

	/* Switch to R3 */
	N(R3); Q("-i5");	/* i3 should be kept alive until i5 dies */
	Q("-i5"); Q("-i5");	/* again, initpass should keep trying to kill i5 */
	K(I5); Q("-i3");	/* i5 died, ok to kill i3 now */
	A(currlevel == R1);	/* waiting for i3 to die */
	K(I3); Q("");
	A(currlevel == R3);

	return 0;
}
