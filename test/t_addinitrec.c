#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;
extern int addinitrec(struct fileblock* fb, char* name, char* runlvl, char* flags, char* cmd, int exe);
extern int mmapblock(struct memblock* m, int length);

int main(void)
{
	int off = sizeof(struct config) + sizeof(struct scratch) + 10;
	struct initrec* pptr;
	struct fileblock fb = {
		.name = "(none)",
		.line = 0,
		.buf = NULL,
		.ls = NULL,
		.le = NULL
	};

	/* mmap a decidedly too small block, and set pointer somewhere in the middle
	   to check how addinitrec will deal with the situation */
	mmapblock(&newblock, off + 7);
	newblock.ptr = off;
	memset(newblock.addr, newblock.len, 0x00);

	T(addinitrec(&fb, "foo", "12", "", strdup("/bin/sh -c true"), 0));
	pptr = blockptr(&newblock, SCR->newend, struct initrec*);

	/* make sure the pointer is ok */
	B(newblock, pptr);

	/* the structure itself should be initialized */
	S(pptr->name, "foo");
	A(pptr->flags == 0);
	A(pptr->rlvl == ( (1<<1) | (1<<2) ));
	A(pptr->pid == 0);
	A(pptr->lastrun == 0);
	A(pptr->lastsig == 0);

	return 0;
}
