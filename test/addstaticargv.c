#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

extern struct nblock newblock;
extern int mmapblock(int len);

extern int addstaticargv(const char** args, int n);

#define count(a) (sizeof(a)/sizeof(*a))
#define to_offset(ptr) ((offset)((void*)ptr - NULL))

int main(void)
{
	int start = 10;

	if(mmapblock(start))
		return -1;

	const char* arg[] = { "foo", "bar" };
	int off = newblock.ptr;

	int ret = addstaticargv(arg, 2);
	ASSERT(ret == 0);
	INTEQUALS(newblock.ptr, start + 3*sizeof(void*) + 8);

	char** ptr = newblockptr(off, char**);
	ASSERT(ptr[0] == NULL + off + (count(arg)+1)*sizeof(char*));
	char* s1 = newblockptr(to_offset(ptr[0]), char*);
	char* s2 = newblockptr(to_offset(ptr[1]), char*);
	STREQUALS(s1, "foo");
	STREQUALS(s2, "bar");
	ASSERT(to_offset(ptr[2]) == 0);

	return 0;
}
