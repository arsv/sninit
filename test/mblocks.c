#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>

#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

extern struct nblock newblock;

extern int mmapblock(int size);
extern offset extendblock(int size);
extern void munmapblock(void);

int main(void)
{
	T(mmapblock(17));
	A(newblock.len >= 17);
	A(newblock.ptr == 17);
	int page = newblock.len;

	/* Try reading within the block */
	A(*newblockptr(0, char*) == 0x00);
	A(*newblockptr(newblock.len - 1, char*) == 0x00);

	/* Trigger new page allocation */
	int ptr = newblock.len - 10;
	A(ptr > 0);	/* sanity check, page should be 4KB or more */
	newblock.ptr = ptr;
	offset off = extendblock(20);
	A(off > 0);
	A(off == ptr);
	A(newblock.len == 2*page);
	/* Try reading and writing there */
	*newblockptr(off + 19, char*) = 0x17;
	A(*newblockptr(off + 19, char*) == 0x17);

	/* Trigger multiple page allocation */
	int pl = newblock.len;
	int pp = newblock.ptr;
	offset largestruct = extendblock(3*page);
	A(largestruct > 0);
	A(newblock.ptr == pp + 3*page);
	A(newblock.len == pl + 3*page);

	return 0;
}
