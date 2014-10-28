#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;
struct memblock scratchblock;

extern int mmapblock(struct memblock* m, int size);
extern int scratchptr(offset listptr, offset ptr);

int main(void)
{
	int listoff = 17;
	struct ptrlist* list;
	struct ptrnode* node;

	/* Skip some bytes at the start, place ptrlist,
	   and move block pointer over */
	int listlen = sizeof(struct ptrlist);
	mmapblock(&scratchblock, listlen + 100);
	memset(scratchblock.addr, listlen + 100, 0);
	scratchblock.ptr = listoff + listlen;

	/* Sanity check, the list should be empty */
	list = blockptr(&scratchblock, listoff, struct ptrlist*);
	A(list->head == 0);
	A(list->last == 0);
	A(list->count == 0);

	/* First pointer, head == last */
	int test1ptr = 0x0000ABCD;
	int node1ptr = scratchblock.ptr;
	T(scratchptr(listoff, test1ptr));
	list = blockptr(&scratchblock, listoff, struct ptrlist*);
	A(list->head == node1ptr);
	A(list->last == node1ptr);
	A(list->count == 1);
	A(list->last > listoff);
	A(list->last == scratchblock.ptr - sizeof(struct ptrnode));
	node = blockptr(&scratchblock, list->last, struct ptrnode*);
	A(node->next == 0);
	A(node->ptr == test1ptr);

	/* Second pointer, head < last */
	int test2ptr = 0xFEFE;
	int node2ptr = scratchblock.ptr;
	T(scratchptr(listoff, test2ptr));
	list = blockptr(&scratchblock, listoff, struct ptrlist*);
	A(list->head == node1ptr);
	A(list->last == node2ptr);
	A(list->count == 2);
	A(list->last > node1ptr);
	A(list->last == scratchblock.ptr - sizeof(struct ptrnode));
	node = blockptr(&scratchblock, list->last, struct ptrnode*);
	A(node->next == 0);
	A(node->ptr == test2ptr);

	/* Make sure nodes have been linked properly */
	node = blockptr(&scratchblock, node1ptr, struct ptrnode*);
	A(node->next == node2ptr);
	A(node->ptr == test1ptr);

	return 0;
}
