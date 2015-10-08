#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>

#include "../init.h"
#include "../init_conf.h"
#include "test.h"

extern struct newblock nblock;

extern int mmapblock(int size);
extern offset extendblock(int size);
extern void munmapblock(void);

int main(void)
{
	T(mmapblock(17));
	A(nblock.len >= 17);
	A(nblock.ptr == 17);
	int page = nblock.len;

	/* Try reading within the block */
	A(*newblockptr(0, char*) == 0x00);
	A(*newblockptr(nblock.len - 1, char*) == 0x00);

	/* Trigger new page allocation */
	int ptr = nblock.len - 10;
	A(ptr > 0);	/* sanity check, page should be 4KB or more */
	nblock.ptr = ptr;
	offset off = extendblock(20);
	A(off > 0);
	A(off == ptr);
	A(nblock.len == 2*page);
	/* Try reading and writing there */
	*newblockptr(off + 19, char*) = 0x17;
	A(*newblockptr(off + 19, char*) == 0x17);

	/* Trigger multiple page allocation */
	int pl = nblock.len;
	int pp = nblock.ptr;
	offset largestruct = extendblock(3*page);
	A(largestruct > 0);
	A(nblock.ptr == pp + 3*page);
	A(nblock.len == pl + 3*page);

	return 0;
}
