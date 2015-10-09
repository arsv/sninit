#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

extern struct nblock newblock;
extern struct fblock fileblock;

extern int addinitrec(char* name, char* rlvl, char* cmd, int exe);
extern int mmapblock(int length);

int main(void)
{
	struct ptrnode* nptr;
	struct initrec* pptr;

	fileblock.name = "nofile";
	fileblock.line = 0;

	/* mmap a decidedly too small block, and set pointer somewhere in the middle
	   to check how addinitrec will deal with the situation */
	int dynhead = sizeof(struct config) + sizeof(struct scratch);
	ZERO(mmapblock(dynhead + 17));
	newblock.ptr = dynhead + 10;
	memset(newblock.addr, 0x00, newblock.len);

	ZERO(addinitrec("foo", heapdup("S12"), heapdup("/bin/sh -c true"), 0));

	/* Sanity check for ptrlist */
	ASSERT(SCR->inittab.head > 0);
	ASSERT(SCR->inittab.head == SCR->inittab.last);
	ASSERT(SCR->inittab.head == dynhead + 10);
	ASSERT(SCR->inittab.count == 1);

	nptr = newblockptr(SCR->inittab.head, struct ptrnode*);
	INBLOCK(newblock, nptr);
	pptr = newblockptr(SCR->inittab.head + sizeof(struct ptrnode), struct initrec*);
	INBLOCK(newblock, pptr);

	/* the structure itself should be initialized */
	STREQUALS(pptr->name, "foo");
	ASSERT(pptr->flags == 0);
	ASSERT(pptr->rlvl == ( (1<<1) | (1<<2) ));
	ASSERT(pptr->pid == 0);
	ASSERT(pptr->lastrun == 0);
	ASSERT(pptr->lastsig == 0);

	/* One more initrec */
	ZERO(addinitrec("bar", heapdup("S234"), heapdup("/sbin/httpd"), 0));

	/* Sanity check for ptrlist */
	ASSERT(SCR->inittab.head < SCR->inittab.last);
	ASSERT(SCR->inittab.count == 2);

	/* Same stuff, now with .last instead of .head */
	nptr = newblockptr(SCR->inittab.last, struct ptrnode*);
	INBLOCK(newblock, nptr);
	pptr = newblockptr(SCR->inittab.last + sizeof(struct ptrnode), struct initrec*);
	INBLOCK(newblock, pptr);

	/* the structure itself should be initialized */
	STREQUALS(pptr->name, "bar");
	ASSERT(pptr->flags == 0);
	ASSERT(pptr->rlvl == ( (1<<2) | (1<<3) | (1<<4) ));
	ASSERT(pptr->pid == 0);
	ASSERT(pptr->lastrun == 0);
	ASSERT(pptr->lastsig == 0);
	
	return 0;
}
