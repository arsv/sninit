#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "../init.h"
#include "../init_conf.h"

extern struct newblock nblock;

extern int mmapblock(int size);
extern int addstring(const char* s);

int main(void)
{
	mmapblock(10);

	nblock.ptr = 0;
	A(addstring("abc") == 0);
	A(((char*)nblock.addr)[4] == '\0');
	S((char*)nblock.addr, "abc");

	return 0;
}
