#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;
struct memblock scratchblock;

extern int addinitrec(struct fileblock* fb, char* name, char* runlvl, char* flags, char* cmd, int exe);
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
	T(mmapblock(&newblock, sizeof(struct config) + 17));
	newblock.ptr = sizeof(struct config) + 10;
	memset(newblock.addr, newblock.len, 0x00);

	T(mmapblock(&scratchblock, 200));
	scratchblock.ptr += sizeof(struct scratch);
	memset(scratchblock.addr, scratchblock.len, 0x00);

	T(addinitrec(&fb, "foo", "12", "", strdup("/bin/sh -c true"), 0));

	/* Sanity check for ptrlist */
	A(SCR->inittab.head > 0);
	A(SCR->inittab.head == SCR->inittab.last);
	A(SCR->inittab.head == scratchblock.ptr - sizeof(struct ptrnode));
	A(SCR->inittab.count == 1);

	/* This is pretty ugly, but also inevitable. SCR->inittab.head is
	   offset of the first ptrnode, which in turn contains the offset
	   of the added initrec. */
	nptr = blockptr(&scratchblock, SCR->inittab.head, struct ptrnode*);
	B(scratchblock, nptr);
	pptr = blockptr(&newblock, nptr->ptr, struct initrec*);
	B(newblock, pptr);

	/* the structure itself should be initialized */
	S(pptr->name, "foo");
	A(pptr->flags == 0);
	A(pptr->rlvl == ( (1<<1) | (1<<2) ));
	A(pptr->pid == 0);
	A(pptr->lastrun == 0);
	A(pptr->lastsig == 0);

	/* One more initrec */
	T(addinitrec(&fb, "bar", "234", "", strdup("/sbin/httpd"), 0));

	/* Sanity check for ptrlist */
	A(SCR->inittab.head < SCR->inittab.last);
	A(SCR->inittab.last == scratchblock.ptr - sizeof(struct ptrnode));
	A(SCR->inittab.count == 2);

	/* Same stuff, now with .last instead of .head */
	nptr = blockptr(&scratchblock, SCR->inittab.last, struct ptrnode*);
	B(scratchblock, nptr);
	pptr = blockptr(&newblock, nptr->ptr, struct initrec*);
	B(newblock, pptr);

	/* the structure itself should be initialized */
	S(pptr->name, "bar");
	A(pptr->flags == 0);
	A(pptr->rlvl == ( (1<<2) | (1<<3) | (1<<4) ));
	A(pptr->pid == 0);
	A(pptr->lastrun == 0);
	A(pptr->lastsig == 0);
	
	return 0;
}
