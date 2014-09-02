/* Routines here place/copy data to newblock and scratchblock.

   Due to strict limitation on what is placed in each of the two blocks,
   most exported functions have no struct memblock argument.
   The caller (addinitrec and addeviron) should not know this
   most of the time anyway. A simple rule is that add* calls
   write to newblock while scratch* write to scratchblock.
 
   Offsets carry neither type nor base block information.
   Care must be taken to use the right memblock.
   The right block is newblock in all cases except:
   	struct ptrnode.next
	struct ptrlist.head
	struct ptrlist.last
   which reference scratchblock instead. */

#include <string.h>
#include "init.h"
#include "init_conf.h"

extern struct memblock newblock;
extern struct memblock scratchblock;

extern int mextendblock(struct memblock* m, int size);

offset addstruct(struct memblock* m, int size, int extra)
{
	if(mextendblock(m, size + extra))
		return -1;

	offset ret = m->ptr;
	m->ptr += size;

	return ret;
}

/* Copy $string to m (adjusting ptr accordingly) and return offset
   at which the string was placed in m.
   Memblock is assumed to have enough space to hold the string. */
static int copystring(struct memblock* m, const char* string, int len)
{
	int ptr = m->ptr;
	memcpy(m->addr + ptr, string, len);
	*(blockptr(m, m->ptr + len, char*)) = '\0';
	m->ptr += len + 1;
	return ptr;
}

/* This is only used for environment variables.
   Since copystring moves ptr, addstruct should not be used here. */
int addstring(const char* string)
{
	int len = strlen(string);

	if(mextendblock(&newblock, len))
		return -1;

	return copystring(&newblock, string, len);
}

/* add*array() functions are used to lay out initrec.argv[]
   in newblock. Source strings can be counted in place if needed,
   but memory required for pointers array is only allocated here.

   Source strings are usually in resp. fileblock and must be copied
   to newblock. 
 
   Both functions return 0 or -1. There's no need to know offset
   of the pointers array since it's always laid at the end of
   struct initrec. */

/* (char* a, char* b, char* c, ...) */
/* Make [ a, b, c, ... ] into an argv-style structure */
int addstrargarray(const char* args[])
{
	int argc = 0;
	int argl = 0;
	const char** p;
	offset po;

	/* see how much space do we need */
	for(p = args; *p; p++, argc++)
		argl += strlen(*p);

	/* allocate the space */
	if((po = addstruct(&newblock, (argc+1)*sizeof(char*), argc + argl)) < 0)
		return -1;
	char** pa = blockptr(&newblock, po, char**);

	/* copy argv[] elements, filling pointers array */
	for(p = args; *p && argc > 0; p++, argc--)
		*(pa++) = NULL + copystring(&newblock, *p, strlen(*p));
	/* terminate pointer array */
	while(argc-- >= 0)
		*(pa++) = NULL;

	return 0;
}

static inline int strlenupto(const char* str, const char* end)
{
	const char* p;
	for(p = str; *p && p < end; p++);
	return p - str;
}

/* Treating str as n concatenated 0-terminated lines, append
   argv-like structure to newblock.
   The pointers array is always NULL-terminated.
   See prepargv() for how an array like this is formed.

   Like with addstrargarray, there's no need to return
   resulting structure offset. */
int addstringarray(int n, const char* str, const char* end)
{
	offset po;
	if((po = addstruct(&newblock, (n+1)*sizeof(char*), (end - str))) < 0)
		return -1;
	char** pa = blockptr(&newblock, po, char**);

	/* the first element is always there — it's the string itself */
	int i, l; const char* p;
	for(i = 0, p = str; i < n && p < end; i++) {
		l = strlenupto(p, end);
		*(pa++) = NULL + copystring(&newblock, p, l);
		p += l + 1;
	}; while(i++ <= n) {
		*(pa++) = NULL;
	}
	
	return 0;
}

/* Make type* array[] style structure in newblock from a ptrlist
   located at listoff in scratchblock. The array is NULL-terminated
   at the back and/or at the front.
   (inittab needs front NULL for reverse pass in initpass)

   Because the pointers are only available when all the data has been placed,
   the pointer array ends up after the actual data in newblock.  */

int addptrsarray(offset listoff, int terminate)
{
	struct memblock* m = &newblock;
	struct memblock* b = &scratchblock;
	struct ptrlist* list = blockptr(b, listoff, struct ptrlist*);

	int rem = list->count;
	int ptrn = rem;
	void** ptrs;
	struct ptrnode* node;
	offset nodeoff;		/* in scratchblock */
	offset ptrsoff;		/* in newblock */

	if(rem < 0) return -1;

	if(terminate & NULL_FRONT) ptrn++;
	if(terminate & NULL_BACK ) ptrn++;

	if((ptrsoff = addstruct(&newblock, ptrn*sizeof(void*), 0)) < 0)
		return -1;

	ptrs = blockptr(m, ptrsoff, void**);

	/* leading NULL pointer is used as terminator when traversing inittab backwards */
	if(terminate & NULL_FRONT) {
		*(ptrs++) = NULL;
		ptrsoff += sizeof(void*);
	}

	/* set up offsets; repoiting will happen later */
	for(nodeoff = list->head; rem && nodeoff; rem--) {
		node = blockptr(b, nodeoff, struct ptrnode*);
		*(ptrs++) = NULL + node->ptr;
		nodeoff = node->next;
	} if(rem)
		/* less elements than expected; this may break initpass, so let's not take chances */
		return -1;

	if(terminate & NULL_BACK)
		*ptrs = NULL;

	return ptrsoff;
}

/* Add ptr to either scratch.inittab (listptr = TABLIST) or scratch.env (ENVLIST) */
/* Passing arbitrary listptr here is a very bad idea. */
int scratchptr(offset listptr, offset ptr)
{
	offset nodeptr = addstruct(&scratchblock, sizeof(struct ptrnode), 0);

	if(nodeptr < 0)
		return -1;

	/* This can only be done *after* extending the block,
	   since mextendblock can very well move scratchblock.addr */
	struct ptrnode* node = blockptr(&scratchblock, nodeptr, struct ptrnode*);
	struct ptrlist* list = blockptr(&scratchblock, listptr, struct ptrlist*);

	if(!list->head)
		list->head = nodeptr;
	if(list->last)
		blockptr(&scratchblock, list->last, struct ptrnode*)->next = nodeptr;

	node->next = 0;
	node->ptr = ptr;

	list->last = nodeptr;
	list->count++;

	return 0;
}

/* This is called during initrec parsing, way before NCF->inittab array
   is formed. So it can't use NCF->inittab. Instead, it should use
   SCR->inittab (which is offset list) to find location of entries added
   so far. */
int checkdupname(const char* name)
{
	offset po = SCR->inittab.head;
	struct ptrnode* n;
	struct initrec* p;

	while(po) {
		n = blockptr(&scratchblock, po, struct ptrnode*);
		p = blockptr(&newblock, n->ptr, struct initrec*);

		if(p->name[0] && !strcmp(p->name, name))
			return -1;

		po = n->next;
	}

	return 0;
}
