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
void addstring(void) { };

extern int setrunlevels(struct fileblock* fb, unsigned short* rlvl, char* runlevels);

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

void test(char* runlevels, int mask)
{
	struct fileblock fb = {
		.name = "(none)",
		.line = 0,
		.buf = NULL,
		.ls = NULL,
		.le = NULL,
		.rlvl = (PRIMASK & ~1)
	};
	unsigned short rlvl;

	printf("%-6s", runlevels);
	if(setrunlevels(&fb, &rlvl, runlevels)) {
		printf("failed\n");
	} else if(rlvl != mask) {
		printf("got 0x%04X expected 0x%04X\n", rlvl, mask);
	} else {
		printf("ok 0x%04X\n", rlvl);
	}
}

int main(void)
{
	test("12",	R1 | R2);
	test("12a",	R1 | R2 | Ra);
	test("12af",	R1 | R2 | Ra | Rf);
	test("9",	R9);
	test("0",	R0);
	test("a",	Ra | (PRIMASK & ~1));
	test("af",	Ra | Rf | (PRIMASK & ~1));
	test("",	(PRIMASK & ~1));

	// negation
	test("~",	1);
	test("~789",	R0 | R1 | R2 | R3 | R4 | R5 | R6);
	test("~123a",	R0 | R4 | R5 | R6 | R7 | R8 | R9 | Ra);

	return 0;
}
