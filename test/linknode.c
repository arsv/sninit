#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

struct nblock newblock;

extern offset extendblock(int size);
extern int mmapblock(int size);
extern int linknode(offset listptr, offset ptr);

NOCALL(setrunflags);

int main(void)
{
	int listoff = 17;
	struct ptrlist* list;
	struct ptrnode* node;

	/* Skip some bytes at the start, place ptrlist,
	   and move block pointer over */
	int listlen = sizeof(struct config) + sizeof(struct ptrlist);
	if(mmapblock(listlen + 100))
		return -1;
	newblock.ptr = listoff + listlen;

	/* Sanity check, the list should be empty */
	list = newblockptr(listoff, struct ptrlist*);
	A(list->head == 0);
	A(list->last == 0);
	A(list->count == 0);

	/* First pointer, head == last */
	int node1ptr = extendblock(sizeof(struct ptrnode));
	T(linknode(listoff, node1ptr));
	list = newblockptr(listoff, struct ptrlist*);
	Eq(list->head, node1ptr, "%i");
	Eq(list->last, node1ptr, "%i");
	Eq(list->count, 1, "%i");
	A(list->last > listoff);
	Eq(list->last, newblock.ptr - sizeof(struct ptrnode), "%i");
	node = newblockptr(list->last, struct ptrnode*);
	A(node->next == 0);

	/* Second pointer, head < last */
	int node2ptr = extendblock(sizeof(struct ptrnode));
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
