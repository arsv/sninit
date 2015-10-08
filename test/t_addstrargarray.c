#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

extern struct newblock nblock;
extern int mmapblock(int len);

extern int addstrargarray(const char** args, int n);

#define count(a) (sizeof(a)/sizeof(*a))
#define to_offset(ptr) ((offset)((void*)ptr - NULL))

int main(void)
{
	int start = 10;

	if(mmapblock(start))
		return -1;

	const char* arg[] = { "foo", "bar" };
	int off = nblock.ptr;

	int ret = addstrargarray(arg, 2);
	A(ret == 0);
	Eq(nblock.ptr, start + 3*sizeof(void*) + 8, "%i");

	char** ptr = newblockptr(off, char**);
	A(ptr[0] == NULL + off + (count(arg)+1)*sizeof(char*));
	char* s1 = newblockptr(to_offset(ptr[0]), char*);
	char* s2 = newblockptr(to_offset(ptr[1]), char*);
	S(s1, "foo");
	S(s2, "bar");
	A(to_offset(ptr[2]) == 0);

	return 0;
}
