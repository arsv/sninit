#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

extern struct newblock nblock;
extern struct fileblock fblock;

extern int addinitrec(char* name, char* rlvl, char* cmd, int exe);
extern int mmapblock(int length);

int main(void)
{
	struct ptrnode* nptr;
	struct initrec* pptr;

	fblock.name = "nofile";
	fblock.line = 0;

	/* mmap a decidedly too small block, and set pointer somewhere in the middle
	   to check how addinitrec will deal with the situation */
	int dynhead = sizeof(struct config) + sizeof(struct scratch);
	T(mmapblock(dynhead + 17));
	nblock.ptr = dynhead + 10;
	memset(nblock.addr, 0x00, nblock.len);

	T(addinitrec("foo", heapdup("S12"), heapdup("/bin/sh -c true"), 0));

	/* Sanity check for ptrlist */
	A(SCR->inittab.head > 0);
	A(SCR->inittab.head == SCR->inittab.last);
	A(SCR->inittab.head == dynhead + 10);
	A(SCR->inittab.count == 1);

	nptr = newblockptr(SCR->inittab.head, struct ptrnode*);
	B(nblock, nptr);
	pptr = newblockptr(SCR->inittab.head + sizeof(struct ptrnode), struct initrec*);
	B(nblock, pptr);

	/* the structure itself should be initialized */
	S(pptr->name, "foo");
	A(pptr->flags == 0);
	A(pptr->rlvl == ( (1<<1) | (1<<2) ));
	A(pptr->pid == 0);
	A(pptr->lastrun == 0);
	A(pptr->lastsig == 0);

	/* One more initrec */
	T(addinitrec("bar", heapdup("S234"), heapdup("/sbin/httpd"), 0));

	/* Sanity check for ptrlist */
	A(SCR->inittab.head < SCR->inittab.last);
	A(SCR->inittab.count == 2);

	/* Same stuff, now with .last instead of .head */
	nptr = newblockptr(SCR->inittab.last, struct ptrnode*);
	B(nblock, nptr);
	pptr = newblockptr(SCR->inittab.last + sizeof(struct ptrnode), struct initrec*);
	B(nblock, pptr);

	/* the structure itself should be initialized */
	S(pptr->name, "bar");
	A(pptr->flags == 0);
	A(pptr->rlvl == ( (1<<2) | (1<<3) | (1<<4) ));
	A(pptr->pid == 0);
	A(pptr->lastrun == 0);
	A(pptr->lastsig == 0);
	
	return 0;
}
