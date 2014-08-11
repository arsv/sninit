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

struct initrec lg = { .pid = 3, .name = "lg", .rlvl = R3 | R0, .flags = 0 };
struct initrec s1 = { .pid = 1, .name = "s1", .rlvl = R3, .flags = 0 };
struct initrec s2 = { .pid = 2, .name = "s2", .rlvl = R3, .flags = 0 };
struct initrec um = { .pid = 0, .name = "um", .rlvl = 0, .flags = C_ONCE | C_WAIT };

struct initrec* testinittab[] = { NULL, &lg, &s1, &s2, &um, NULL };
struct config testconfig = { .inittab = testinittab + 1, .initnum = sizeof(testinittab)/sizeof(void*) - 2 };

struct config* cfg = &testconfig;

#define Q(t) { reset(); initpass(); S(passlog, t); }
#define Qq(t) Q(t); Q("")
#define D(i) died(&i)
#define K(i) killed(&i)
#define N(r) nextlevel = r

int main(void)
{
	currlevel = R3;
	nextlevel = R0;
	state = 0;

	/* start killing daemons */
	Q("-s1-s2");
	A(currlevel == R3);

	/* both should die before anything else happens */
	D(s1); Q("-s2");
	D(s2); Q("");
	A(currlevel == R0);
	A(nextlevel == 0);

	/* now try to kill syslog */
	Q("-lg");
	D(lg); Q("+um");
	D(um); Q("");
	A(currlevel == 0);

	return 0;
}
