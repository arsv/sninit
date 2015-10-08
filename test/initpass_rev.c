#include "../init.h"
#include "_test.h"

extern int state;
extern int currlevel;
extern int nextlevel;
extern int sublevels;

extern char passlog[];
extern void reset(void);
extern void died(struct initrec* p);
extern void killed(struct initrec* p);
extern void initpass(void);

#define R0 (1<<0)
#define R1 (1<<1)

struct initrec I0 = { .pid = 0, .name = "i0", .rlvl = R1, .flags = C_ONCE };
struct initrec I1 = { .pid = 0, .name = "i1", .rlvl = R1, .flags = 0 };
struct initrec I2 = { .pid = 0, .name = "i2", .rlvl = R1, .flags = C_WAIT };
struct initrec I3 = { .pid = 0, .name = "i3", .rlvl = R1, .flags = C_ONCE | C_WAIT };
struct initrec I4 = { .pid = 0, .name = "i4", .rlvl = R1, .flags = 0 };

struct initrec* testinittab[] = { NULL, &I0, &I1, &I2, &I3, &I4, NULL };
struct config testconfig = { .inittab = testinittab + 1, .initnum = sizeof(testinittab)/sizeof(void*) - 2 };

struct config* cfg = &testconfig;

#define Q(t) { reset(); initpass(); S(passlog, t); }
#define Qq(t) Q(t); Q("")
#define D(i) died(&i)
#define K(i) killed(&i)
#define N(r) nextlevel = r

int main(void)
{
	currlevel = R0;
	nextlevel = R1;
	state = 0;

	/* This is to make sure "hold" option (i.e. C_WAIT w/o C_ONCE) is handled correctly */

	Qq("+i0+i1+i2");	/* start everything up to C_ONCE | C_WAIT entry */
	D(I0); Q("+i3");	/* i3 waits for all o-type entries, then starts */
	D(I1); Q("+i1");	/* s-entry is eligible to be restarted */
	D(I2); Q("+i2");	/* h-entry is eligible to be restarted */
	Q("");			/* nothing to do while w-type i3 is running */
	A(currlevel != R1);	/* should NOT switch until I2 exist */
	D(I3); Q("+i4");	/* w-type done, ok to start the rest */
	Q("");			/* nothing to do yet */
	A(currlevel == R1);	/* and since that was the last ONCE entry, we're in R1 */

	return 0;
}
