#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

/* Empty symbols to keep the linker happy */
struct memblock newblock;
void addstrargarray(void) { };
void addstringarray(void) { };
void mextendblock(void) { };

extern int setrunlevels(struct initrec* entry, char* runlevels, struct fileblock* fb);

#define R0 (1<<0)
#define R1 (1<<1)
#define R2 (1<<2)
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
		.le = NULL
	};
	struct initrec entry;

	printf("%-6s", runlevels);
	if(setrunlevels(&entry, runlevels, &fb)) {
		printf("failed\n");
	} else if(entry.rlvl != mask) {
		printf("got 0x%04X expected 0x%04X\n", entry.rlvl, mask);
	} else {
		printf("ok 0x%04X\n", entry.rlvl);
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
	return 0;
}
