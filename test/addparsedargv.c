#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

int addparsedargv(char* str);

#define asoffset(ptr) ((offset)((void*)ptr - NULL))

int main(void)
{
	int start = 10;

	if(mmapblock(start))
		return -1;

	char args[] = "blah  foo\tbar  ";
	int off = newblock.ptr;

	int ret = addparsedargv(args);
	ASSERT(ret == 0);
	INTEQUALS(newblock.ptr, start + 4*sizeof(char*) + 13);

	char** ptr = newblockptr(off, char**);
	ASSERT(ptr[0] == NULL + off + 4*sizeof(char*));
	char* s1 = newblockptr(asoffset(ptr[0]), char*);
	char* s2 = newblockptr(asoffset(ptr[1]), char*);
	char* s3 = newblockptr(asoffset(ptr[2]), char*);
	STREQUALS(s1, "blah");
	STREQUALS(s2, "foo");
	STREQUALS(s3, "bar");
	ASSERT(asoffset(ptr[3]) == 0);

	return 0;
}
