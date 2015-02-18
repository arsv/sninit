#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;

extern int addinitrec(struct fileblock* fb, char* name, char* rlvl, char* cmd, int exe);
extern int mmapblock(struct memblock* m, int length);

int main(void)
{
	struct ptrnode* nptr;
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
	int dynhead = sizeof(struct config) + sizeof(struct scratch);
	T(mmapblock(&newblock, dynhead + 17));
	newblock.ptr = dynhead + 10;
	memset(newblock.addr, newblock.len, 0x00);

	T(addinitrec(&fb, "foo", "12", heapdup("/bin/sh -c true"), 0));

	/* Sanity check for ptrlist */
	A(SCR->inittab.head > 0);
	A(SCR->inittab.head == SCR->inittab.last);
	A(SCR->inittab.head == dynhead + 10);
	A(SCR->inittab.count == 1);

	nptr = newblockptr(SCR->inittab.head, struct ptrnode*);
	B(newblock, nptr);
	pptr = newblockptr(SCR->inittab.head + sizeof(struct ptrnode), struct initrec*);
	B(newblock, pptr);

	/* the structure itself should be initialized */
	S(pptr->name, "foo");
	A(pptr->flags == (C_DOF | C_DTF));
	A(pptr->rlvl == ( (1<<1) | (1<<2) ));
	A(pptr->pid == 0);
	A(pptr->lastrun == 0);
	A(pptr->lastsig == 0);

	/* One more initrec */
	T(addinitrec(&fb, "bar", "234", heapdup("/sbin/httpd"), 0));

	/* Sanity check for ptrlist */
	A(SCR->inittab.head < SCR->inittab.last);
	A(SCR->inittab.count == 2);

	/* Same stuff, now with .last instead of .head */
	nptr = newblockptr(SCR->inittab.last, struct ptrnode*);
	B(newblock, nptr);
	pptr = newblockptr(SCR->inittab.last + sizeof(struct ptrnode), struct initrec*);
	B(newblock, pptr);

	/* the structure itself should be initialized */
	S(pptr->name, "bar");
	A(pptr->flags == (C_DOF | C_DTF));
	A(pptr->rlvl == ( (1<<2) | (1<<3) | (1<<4) ));
	A(pptr->pid == 0);
	A(pptr->lastrun == 0);
	A(pptr->lastsig == 0);
	
	return 0;
}
