#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct nblock newblock;

void* inittab;
void* cfg;
int currlevel;

NOCALL(readinittab);
NOCALL(readinitdir);
NOCALL(findentry);
NOCALL(addptrsarray);
NOCALL(levelmatch);
NOCALL(addstruct);

extern int mmapblock(int size);
extern void rewireptrsarray(void** a);

int main(void)
{
	T(mmapblock(10));

	/* First, construct required pointers structure */
	int testargc = 3;
	char* testargv[] = { "arg1", "arg2", "arg3", NULL };
	/* must be offset from the start of buf */
	/* not to let repoint() think it's a NULL pointer */
	int argvoff = 17;
	int argc = testargc;

	int stringsoff = argvoff + (argc+1)*sizeof(char*);

	char* sp;
	int spoff = stringsoff;
	char** srcp = testargv;
	char** dstp = newblockptr(argvoff, char**);
	int args[argc];
	int* argp = args;
	while(*srcp) {
		sp = newblockptr(spoff, char*);
		strcpy(sp, *srcp);
		*(dstp++) = NULL + spoff;
		*(argp++) = spoff;		// save the offset the string was placed at
		spoff += strlen(*(srcp++)) + 1;	// skip over the newly placed string
	}
	newblock.ptr = spoff;

	/* Now try to repoint the array... */
	char** argv = newblock.addr + argvoff;
	rewireptrsarray((void**) argv);

	/* ..and check the results */
	A(argv == newblockptr(argvoff, char**));
	A(argv[0] == newblockptr(args[0], char*));
	A(argv[1] == newblockptr(args[1], char*));
	A(argv[2] == newblockptr(args[2], char*));
	A(argv[3] == NULL);

	return 0;
}
