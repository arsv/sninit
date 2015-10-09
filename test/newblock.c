#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

extern struct nblock newblock;

extern int mmapblock(int size);
extern offset extendblock(int size);
extern void munmapblock(void);

int main(void)
{
	ZERO(mmapblock(17));
	ASSERT(newblock.len >= 17);
	ASSERT(newblock.ptr == 17);
	int page = newblock.len;

	/* Try reading within the block */
	ASSERT(*newblockptr(0, char*) == 0x00);
	ASSERT(*newblockptr(newblock.len - 1, char*) == 0x00);

	/* Trigger new page allocation */
	int ptr = newblock.len - 10;
	ASSERT(ptr > 0);	/* sanity check, page should be 4KB or more */
	newblock.ptr = ptr;
	offset off = extendblock(20);
	ASSERT(off > 0);
	INTEQUALS(off, ptr);
	INTEQUALS(newblock.len, 2*page);
	/* Try reading and writing there */
	*newblockptr(off + 19, char*) = 0x17;
	ASSERT(*newblockptr(off + 19, char*) == 0x17);

	/* Trigger multiple page allocation */
	int pl = newblock.len;
	int pp = newblock.ptr;
	offset largestruct = extendblock(3*page);
	ASSERT(largestruct > 0);
	INTEQUALS(newblock.ptr, pp + 3*page);
	INTEQUALS(newblock.len, pl + 3*page);

	return 0;
}
