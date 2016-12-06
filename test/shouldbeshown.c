#include <stdlib.h>
#include <stdio.h>
#include "../init.h"

int currlevel;
int nextlevel;
struct config* cfg = NULL;

#define SHOW(fl, rl) {\
	struct initrec ir = { .flags = fl, .rlvl = rl, .pid = 0 };\
	if(shouldbeshown(&ir))\
		printf("%s:%i: OK showing %s %s\n", __FILE__, __LINE__, #fl, #rl);\
	else\
		printf("%s:%i: FAIL not showing %s %s\n", __FILE__, __LINE__, #fl, #rl);\
}

#define HIDE(fl, rl) {\
	struct initrec ir = { .flags = fl, .rlvl = rl, .pid = 0 };\
	if(shouldbeshown(&ir))\
		printf("%s:%i: FAIL showing %s %s\n", __FILE__, __LINE__, #fl, #rl);\
	else\
		printf("%s:%i: OK not showing %s %s\n", __FILE__, __LINE__, #fl, #rl);\
}

#define R1 (1<<1)
#define R2 (1<<2)
#define R3 (1<<3)
#define R4 (1<<4)
#define Ra (1<<0xa)

int levelmatch(struct initrec* p, int level)
{
	return (p->rlvl & level);
}

void warn(const char* fmt, ...)
{
}

int main(void)
{
	currlevel = R3;
	nextlevel = R3;

	SHOW(0,      R3 | R4);
	HIDE(0,      R2 | R1);
	HIDE(C_ONCE, R3);

	currlevel = 1<<2;
	nextlevel = 1<<3;

	SHOW(0,      R3 | R4);
	SHOW(0,      R2);
	HIDE(0,      R4);
	SHOW(C_ONCE, R3);

	return 0;
}
