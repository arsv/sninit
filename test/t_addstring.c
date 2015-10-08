#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "../init.h"
#include "../init_conf.h"

extern struct nblock newblock;

extern int mmapblock(int size);
extern int addstring(const char* s);

int main(void)
{
	mmapblock(10);

	newblock.ptr = 0;
	A(addstring("abc") == 0);
	A(((char*)newblock.addr)[4] == '\0');
	S((char*)newblock.addr, "abc");

	return 0;
}
