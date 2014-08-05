#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;
struct memblock scratchblock;

void* inittab;
void* cfg;
int state;
int currlevel;
void readinittab() { };
void findentry() { };
void addptrsarray() { };

extern int mmapblock(struct memblock* m, int size);
extern void rewireptrsarray(void** a);

int main(void)
{
	T(mmapblock(&newblock, 1024));

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
	char** dstp = blockptr(&newblock, argvoff, char**);
	int args[argc];
	int* argp = args;
	while(*srcp) {
		sp = blockptr(&newblock, spoff, char*);
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
	A(argv == blockptr(&newblock, argvoff, char**));
	A(argv[0] == blockptr(&newblock, args[0], char*));
	A(argv[1] == blockptr(&newblock, args[1], char*));
	A(argv[2] == blockptr(&newblock, args[2], char*));
	A(argv[3] == NULL);

	return 0;
}
