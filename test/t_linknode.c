#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;

extern offset addstruct(struct memblock* m, int size, int extra);
extern int mmapblock(struct memblock* m, int size);
extern int linknode(offset listptr, offset ptr);

int main(void)
{
	int listoff = 17;
	struct ptrlist* list;
	struct ptrnode* node;

	/* Skip some bytes at the start, place ptrlist,
	   and move block pointer over */
	int listlen = sizeof(struct ptrlist);
	mmapblock(&newblock, listlen + 100);
	memset(newblock.addr, listlen + 100, 0);
	newblock.ptr = listoff + listlen;

	/* Sanity check, the list should be empty */
	list = newblockptr(listoff, struct ptrlist*);
	A(list->head == 0);
	A(list->last == 0);
	A(list->count == 0);

	/* First pointer, head == last */
	int node1ptr = addstruct(&newblock, sizeof(struct ptrnode), 0);
	T(linknode(listoff, node1ptr));
	list = newblockptr(listoff, struct ptrlist*);
	A(list->head == node1ptr);
	A(list->last == node1ptr);
	A(list->count == 1);
	A(list->last > listoff);
	A(list->last == newblock.ptr - sizeof(struct ptrnode));
	node = newblockptr(list->last, struct ptrnode*);
	A(node->next == 0);

	/* Second pointer, head < last */
	int node2ptr = addstruct(&newblock, sizeof(struct ptrnode), 0);
	T(linknode(listoff, node2ptr));
	list = newblockptr(listoff, struct ptrlist*);
	A(list->head == node1ptr);
	A(list->last == node2ptr);
	A(list->count == 2);
	A(list->last > node1ptr);
	A(list->last == newblock.ptr - sizeof(struct ptrnode));
	node = newblockptr(list->last, struct ptrnode*);
	A(node->next == 0);

	/* Make sure nodes have been linked properly */
	node = newblockptr(node1ptr, struct ptrnode*);
	A(node->next == node2ptr);

	return 0;
}
