#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

/* Empty symbols to keep the linker happy */
struct memblock newblock;
void addstrargarray(void) { };
void addstringarray(void) { };
void mextendblock(void) { };
void checkdupname(void) { };
void scratchptr(void) { };

extern int setrunflags(struct fileblock* fb, struct initrec* entry, char* flagstring);

#define R0 (1<<0)
#define R1 (1<<1)
#define R2 (1<<2)
#define R3 (1<<3)
#define R4 (1<<4)
#define R5 (1<<5)
#define R6 (1<<6)
#define R7 (1<<7)
#define R8 (1<<8)
#define R9 (1<<9)
#define Ra (1<<0xa)
#define Rf (1<<0xf)

struct fileblock fb = {
	.name = "(none)",
	.line = 0,
	.buf = NULL,
	.ls = NULL,
	.le = NULL
};
struct initrec rec = {
	.rlvl = 0
};

#define sr(str) \
	T(setrunflags(&fb, &rec, str))

#define bb(str) \
	T(!setrunflags(&fb, &rec, str))

#define rl(str, exp) \
	sr(str);\
	Eq(rec.rlvl, exp, "%04X")

#define fl(str, exp) \
	sr(str);\
	Eq(rec.flags, exp, "%04X")

#define rf(str, er, ef) \
	sr(str);\
	Eq(rec.rlvl, er, "%04X");\
	Eq(rec.flags, ef, "%04X")

int main(void)
{
	rl("12",	R1 | R2);
	rl("S12a",	R1 | R2 | Ra);
	rl("12af",	R1 | R2 | Ra | Rf);
	rl("9",		R9);
	rl("0",		R0);
	rl("a",		Ra | (PRIMASK & ~1));
	rl("af",	Ra | Rf | (PRIMASK & ~1));
	rl("",		(PRIMASK & ~1));

	// negation
	rl("~",		R0);
	rl("~789",	R0 | R1 | R2 | R3 | R4 | R5 | R6);
	rl("~123a",	R0 | R4 | R5 | R6 | R7 | R8 | R9 | Ra);

	fl("S12",	0);
	fl("E",		C_ONCE);
	fl("W",		C_ONCE | C_WAIT);
	fl("H",		C_WAIT);

	/* Runlevels and flags together */
	rf("W12",	R1 | R2,	C_ONCE | C_WAIT);

	/* incorrect cases */
	bb("Z");
	bb("?");

	return 0;
}
