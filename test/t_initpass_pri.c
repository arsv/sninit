#include "../init.h"
#include "test.h"

extern int state;
extern int currlevel;
extern int nextlevel;
extern int sublevels;

extern char passlog[];
extern void reset(void);
extern void died(struct initrec* p);
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
struct initrec I3 = { .next = &I4,  .prev = &I2, .pid = 0, .name = "i3", .rlvl = R1,  .flags = 0 };
struct initrec I4 = { .next = &I5,  .prev = &I3, .pid = 0, .name = "i4", .rlvl = R12, .flags = C_ONCE };
struct initrec I5 = { .next = &I6,  .prev = &I4, .pid = 0, .name = "i5", .rlvl = R12, .flags = 0 };
struct initrec I6 = { .next = NULL, .prev = &I5, .pid = 0, .name = "i6", .rlvl = R1a, .flags = 0 };

#define Q(t) { reset(); initpass(); S(passlog, t); }
#define Qq(t) Q(t); Q("")

int main(void)
{
	currlevel = 0;
	nextlevel = R1;
	state = 0;

	Qq("+i0+i1");		/* start o-entries, do NOT start w-entry until both o-entries are dead */

	died(&I1);
	Q("");			/* do not start w-entry until both o-entries are dead */
	died(&I0);
	Qq("+i2");		/* ok, no we can start w-entry, but should not proceed further and wait while it runs */

	died(&I2);
	Q("+i3+i4+i5");		/* w-type done, ok to start the rest */
	A(currlevel != R1);	/* should NOT switch until I4 exist */
	Q("");			/* nothing to do yet */

	died(&I3);		/* it's an s-type entry, got to restrt it */
	Qq("+i3");		

	died(&I4);
	Q("");			/* do not restart it */
	A(currlevel == R1);	/* and do switch levels */

	died(&I5);
	Qq("+i5");		/* s-type, go to restart */

	nextlevel = R2;
	Qq("-i3");		/* got to stop this particular s-entry only */
	A(currlevel == R2);

	nextlevel = R1;
	Qq("+i0+i3");		/* got to re-run I0, and restart I3 */

	return 0;
}
