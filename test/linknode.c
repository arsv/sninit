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
	ASSERT(list->head == 0);
	ASSERT(list->last == 0);
	ASSERT(list->count == 0);

	/* First pointer, head == last */
	int node1ptr = extendblock(sizeof(struct ptrnode));
	ZERO(linknode(listoff, node1ptr));
	list = newblockptr(listoff, struct ptrlist*);
	INTEQUALS(list->head, node1ptr);
	INTEQUALS(list->last, node1ptr);
	INTEQUALS(list->count, 1);
	ASSERT(list->last > listoff);
	INTEQUALS(list->last, newblock.ptr - sizeof(struct ptrnode));
	node = newblockptr(list->last, struct ptrnode*);
	ASSERT(node->next == 0);

	/* Second pointer, head < last */
	int node2ptr = extendblock(sizeof(struct ptrnode));
	ZERO(linknode(listoff, node2ptr));
	list = newblockptr(listoff, struct ptrlist*);
	ASSERT(list->head == node1ptr);
	ASSERT(list->last == node2ptr);
	ASSERT(list->count == 2);
	ASSERT(list->last > node1ptr);
	ASSERT(list->last == newblock.ptr - sizeof(struct ptrnode));
	node = newblockptr(list->last, struct ptrnode*);
	ASSERT(node->next == 0);

	/* Make sure nodes have been linked properly */
	node = newblockptr(node1ptr, struct ptrnode*);
	ASSERT(node->next == node2ptr);

	return 0;
}
