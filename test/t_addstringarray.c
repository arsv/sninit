#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

extern struct newblock nb;

int addstringarray(char* str);
extern int mmapblock(int len);

#define count(a) (sizeof(a)/sizeof(*a))
#define to_offset(ptr) ((offset)((void*)ptr - NULL))

int main(void)
{
	int start = 10;

	if(mmapblock(start))
		return -1;

	char args[] = "blah  foo\tbar  ";
	int off = nb.ptr;

	int ret = addstringarray(args);
	A(ret == 0);
	Eq(nb.ptr, start + 4*sizeof(char*) + 13, "%i");

	char** ptr = newblockptr(off, char**);
	A(ptr[0] == NULL + off + 4*sizeof(char*));
	char* s1 = newblockptr(to_offset(ptr[0]), char*);
	char* s2 = newblockptr(to_offset(ptr[1]), char*);
	char* s3 = newblockptr(to_offset(ptr[2]), char*);
	S(s1, "blah");
	S(s2, "foo");
	S(s3, "bar");
	A(to_offset(ptr[3]) == 0);

	return 0;
}
