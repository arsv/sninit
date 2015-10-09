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
#define R2 (1<<2)
#define R3 (1<<3)
#define R8 (1<<8)
#define R12 (R1|R2)
#define R13 (R1|R3)
#define R123 (R1|R2|R3)
#define Ra (1<<0xa)
#define R1a (R1|Ra)
#define SM SUBMASK

struct initrec sw = { .pid = -1, .name = "sw", .rlvl = 0x02FF, .flags = C_ONCE | C_WAIT };
struct initrec sl = { .pid =  0, .name = "sl", .rlvl = 0x0100, .flags = C_ONCE | C_WAIT };

struct initrec* testinittab[] = { NULL, &sw, &sl, NULL };
struct config testconfig = { .inittab = testinittab + 1, .initnum = 2 };

struct config* cfg = &testconfig;

#define A(e) ASSERT(e)
#define Q(t) { reset(); initpass(); STREQUALS(passlog, t); }
#define Qq(t) Q(t); Q("")
#define D(i) died(&i)
#define K(i) killed(&i)
#define N(r) nextlevel = r

int main(void)
{
	currlevel = R0;
	nextlevel = R1;
	state = 0;

	Q("");
	A(currlevel == R1);

	nextlevel = R8;
	Q("+sl"); D(sl); Q("");
	A(currlevel == R8);

	nextlevel = R1;
	Q("+sw"); D(sw); Q("");
	A(currlevel == R1);

	return 0;
}
