#include <string.h>
#include "init.h"
#include "init_conf.h"
#include "scope.h"

/* All add* functions copy relevant data to newblock,
   possibly extending it, and adjust newblock.ptr accordingly.

   addstruct and addptrsarray return the offset
   the data was placed at, or -1 in case of error.

   addstrargarray and addstringarray only distingluish between
   error and non-error, they are used for argv[] which is always
   laid out after respective initrec, so the actual offset is
   not really relevant. */

extern struct memblock newblock;

export offset addstruct(int size, int extra);
export int addstrargarray(const char* args[]);
export int addstringarray(int n, const char* str, const char* end);
export int addptrsarray(offset listoff, int terminate);
export int checkdupname(const char* name);

extern int mextendblock(struct memblock* m, int size);


offset addstruct(int size, int extra)
{
	if(mextendblock(&newblock, size + extra))
		return -1;

	offset ret = newblock.ptr;
	newblock.ptr += size;

	return ret;
}

/* string is len-long, and may not be 0-terminated */
local int addstring(const char* string, int len)
{
	int ptr = newblock.ptr;
	memcpy(newblock.addr + ptr, string, len);
	*(newblockptr(ptr + len, char*)) = '\0';
	newblock.ptr += len + 1;
	return ptr;
}

/* add*array() functions are used to lay out initrec.argv[]
   Source strings are usually in resp. fileblock and must be copied
   to newblock. */
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
	if((po = addstruct((argc+1)*sizeof(char*), argc + argl)) < 0)
		return -1;
	char** pa = newblockptr(po, char**);

	/* copy argv[] elements, filling pointers array */
	for(p = args; *p && argc > 0; p++, argc--)
		*(pa++) = NULL + addstring(*p, strlen(*p));
	/* terminate pointer array */
	while(argc-- >= 0)
		*(pa++) = NULL;

	return 0;
}

local int strlenupto(const char* str, const char* end)
{
	const char* p;
	for(p = str; *p && p < end; p++);
	return p - str;
}

/* Treating str as n concatenated 0-terminated lines, append a proper
   argv[] structure to newblock. It is assumed that initrec itself
   is already there.

   The pointers array is NULL-terminated, as required by execve(2).

   See prepargv() for how an array like this is formed. */

int addstringarray(int n, const char* str, const char* end)
{
	offset po;
	if((po = addstruct((n+1)*sizeof(char*), (end - str))) < 0)
		return -1;
	char** pa = newblockptr(po, char**);

	/* the first element is always there — it's the string itself */
	int i, l; const char* p;
	for(i = 0, p = str; i < n && p < end; i++) {
		l = strlenupto(p, end);
		*(pa++) = NULL + addstring(p, l);
		p += l + 1;
	}; while(i++ <= n) {
		*(pa++) = NULL;
	}
	
	return 0;
}

/* Make type* array[] style structure in newblock from a ptrlist
   located at listoff in newblock. The array is NULL-terminated
   at the back and/or at the front.
   (inittab needs front NULL for reverse pass in initpass)

   Because the pointers are only available when all the data has
   been placed, the pointer array ends up after the actual data
   in newblock, with back-referencing pointers.
 
   This function is used to lay out config.inittab and config.env,
   but not for initrec.argv which gets a different treatment. */

int addptrsarray(offset listoff, int terminate)
{
	struct ptrlist* list = newblockptr(listoff, struct ptrlist*);

	int rem = list->count;
	int ptrn = rem;
	void** ptrs;
	struct ptrnode* node;
	offset nodeoff;		/* in scratchblock */
	offset ptrsoff;		/* in newblock */

	if(rem < 0) return -1;

	if(terminate & NULL_FRONT) ptrn++;
	if(terminate & NULL_BACK ) ptrn++;

	if((ptrsoff = addstruct(ptrn*sizeof(void*), 0)) < 0)
		return -1;

	ptrs = newblockptr(ptrsoff, void**);

	/* leading NULL pointer is used as terminator when traversing inittab backwards */
	if(terminate & NULL_FRONT) {
		*(ptrs++) = NULL;
		ptrsoff += sizeof(void*);
	}

	/* set up offsets; repoiting will happen later */
	for(nodeoff = list->head; rem && nodeoff; rem--) {
		node = newblockptr(nodeoff, struct ptrnode*);
		*(ptrs++) = NULL + nodeoff + sizeof(struct ptrnode);
		nodeoff = node->next;
	} if(rem)
		/* less elements than expected; this may break initpass, so let's not take chances */
		return -1;

	if(terminate & NULL_BACK)
		*ptrs = NULL;

	return ptrsoff;
}

/* It's an error to have two entries with the same (non-empty) name
   as it makes telinit commands ambiguous, so dupes are checked and
   reported at parsing stage.

   This is called during initrec parsing, way before SCR->inittab array
   is formed. So it can't use SCR->inittab. Instead, it should use
   SCR->inittab (which is offset list) to find location of entries added
   so far. */

int checkdupname(const char* name)
{
	offset po = SCR->inittab.head;
	struct ptrnode* n;
	struct initrec* p;

	while(po) {
		n = newblockptr(po, struct ptrnode*);
		p = newblockptr(po + sizeof(struct ptrnode), struct initrec*);

		if(p->name[0] && !strcmp(p->name, name))
			return -1;

		po = n->next;
	}

	return 0;
}
