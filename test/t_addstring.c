#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "../init.h"
#include "../init_conf.h"

struct memblock newblock;

extern int mmapblock(struct memblock* m, int size);
extern int addstring(const char* s);

int main(void)
{
	mmapblock(&newblock, 1000);

	A(addstring("abc") == 0);
	A(((char*)newblock.addr)[4] == '\0');
	S((char*)newblock.addr, "abc");

	return 0;
}
