#define _GNU_SOURCE
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include "init.h"
#include "init_conf.h"
#include "scope.h"

/* Init uses mmap to allocate memory *and* to read files.
   At any given point, there are at most three mmaped areas: */

struct cfgblock cblock;         /* active configuration */
struct newblock nblock;         /* newly compiled configuration */
struct fileblock fblock;        /* the file being parsed */

/* New configuration is built by appending structures of known size
   to newblock. If reconfiguration succeeds, the blocks are exchanged:
   old cfgblock gets unmmaped and newblock becomes cfgblock.
   In case configuration fails, we just unmmap newblock. */

export int mmapblock(int size);
export offset extendblock(int size);
export void exchangeblocks(void);
export void munmapblock(void);

/* The file block is used to parse inittab or service files linewise.
   Filename is stored in the structure for error reporting.
   The pointer must be usable up until munmapfile is called. */

export int mmapfile(const char* filename, int maxlen);
export int munmapfile(void);
export char* nextline(void);

/* Whenever possible, memory is mmaped in memblockalign increments
   to reduce the number of calls. To make the whole thing testable,
   int is used instead of placing IRALLOC in all relevant calls. */

local int memblockalign = IRALLOC;

/* The block is initially allocated to hold struct config and
   struct scratch. Initrecs are added later with extendblock. */

int mmapblock(int size)
{
	int aligned = size;

	if(size % memblockalign)
		aligned += (memblockalign - size % memblockalign);

	if(nblock.addr) {
		/* This is a relatively unlikely case when a new reconfigure
		   request comes before newblock from the previous one
		   is moved over to cfgblock. In such a case, try to re-use
		   newblock without unmmaping it. Due to the way mextendblock
		   works, the old block must be large enough; if it is not,
		   it's a hard error. */
		if(nblock.len < size)
			return -1;
	} else {
		const int prot = PROT_READ | PROT_WRITE;
		const int flags = MAP_PRIVATE | MAP_ANONYMOUS;

		void* addr = mmap(NULL, aligned, prot, flags, -1, 0);

		if(addr == MAP_FAILED)
			return -1;

		nblock.addr = addr;
		nblock.len = aligned;
	}

	memset(nblock.addr, 0, nblock.len);
	nblock.ptr = size;

	return 0;
}

#ifdef NOMMU
/* On non-MMU targets, MREMAP_MAYMOVE does not really work,
   so we've got to emulate it explicitly. */
static void* mremapnommu(void* oldaddr, size_t oldsize, size_t newsize, int flags)
{
	void* newaddr = mremap(oldaddr, oldsize, newsize, flags);

	if(newaddr != MAP_FAILED)
		return newaddr;

	newaddr = mmap(NULL, newsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if(newaddr == MAP_FAILED)
		return newaddr;

	memcpy(newaddr, oldaddr, oldsize);
	munmap(oldaddr, oldsize);

	return newaddr;
}
#define mremap(oa, os, ns, fl) mremapnommu(oa, os, ns, fl)
#endif

/* nblock.ptr tracks the start of empty space in nblock.
   We make sure there is enough space, and allocate it by moving ptr over. */

offset extendblock(int size)
{
	int ret = nblock.ptr;
	int alloc = size;

	if(nblock.len - nblock.ptr > size)
		goto moveptr;
	
	if(alloc % memblockalign)
		alloc += (memblockalign - alloc % memblockalign);

	/* size is just an int, overflows are possible (but very unlikely) */
	if(alloc < 0 || nblock.len + alloc < 0)
		return -1;

	void* np = mremap(nblock.addr, nblock.len, nblock.len + alloc, MREMAP_MAYMOVE);

	if(np == MAP_FAILED)
		return -1;

	nblock.addr = np;
	nblock.len += alloc;
moveptr:
	nblock.ptr += size;

	return ret;
}

void munmapblock(void)
{
	munmap(nblock.addr, nblock.len);

	nblock.addr = 0;
	nblock.len = 0;
};

/* cfgblock is empty if builtin config is used */

void exchangeblocks(void)
{
	if(cblock.addr)
		munmap(cblock.addr, cblock.len);

	cblock.addr = nblock.addr;
	cblock.len = nblock.len;

	nblock.addr = NULL;
	nblock.len = 0;
};

/* Due to average inittab being about 1-2k, it is always read whole;
   for service files, only the head is mmaped.
   Init makes no distinction between mmap failure and open failure,
   both mean the new inittab won't be used.

	maxlen > 0: error if the file is larger (inittab)
	maxlen < 0: map at most -maxlen first bytes (initdir)

   The result is always 0-terminated. */

int mmapfile(const char* filename, int maxlen)
{
	struct stat st;

	int fd = open(filename, O_RDONLY);
	if(fd < 0)
		retwarn(-1, "can't open %s: %m", filename);

	if(fstat(fd, &st) < 0)
		gotowarn(out, "can't stat %s: %m", filename);

	if(!S_ISREG(st.st_mode))
		gotowarn(out, "%s: not a regular file", filename);
	if(maxlen < 0 && st.st_size > -maxlen)
		/* Because ints are used in lots of places,
		   it is a good idea to avoid loading anything that
		   exceeds 2^31 when compiled into newblock */
		gotowarn(out, "%s: file too large", filename);

	int stm = st.st_size;
	if(maxlen > 0 && stm > maxlen) stm = maxlen;

	/* with one guard byte at the end, to hold \0 */
	const int prot = PROT_READ | PROT_WRITE;
	const int flags = MAP_PRIVATE;
	char* addr = mmap(NULL, stm + 1, prot, flags, fd, 0);

	if(addr == MAP_FAILED)
		gotowarn(out, "%s: mmap failed: %m", filename);

	addr[stm] = '\0';

	fblock.buf = addr;
	fblock.len = stm;
	fblock.ls = NULL;
	fblock.le = NULL;
	fblock.name = filename;
	fblock.line = 0;

	close(fd);
	return 0;

out:	close(fd);
	return -1;
}

int munmapfile(void)
{
	return munmap(fblock.buf, fblock.len);
}

/* The file is mmaped rw and private. We place the pointers
   and terminate the line with \0, overwriting \n. */

char* nextline(void)
{
	char* le = fblock.le;
	char* ls = le ? le + 1 : fblock.buf;
	char* end = fblock.buf + fblock.len;

	if(ls >= end) return NULL;

	for(le = ls; le < end && *le != '\n'; le++)
		; /* clang is full of hatred towards elegant concise expressions */
	*le = '\0';

	fblock.ls = ls;
	fblock.le = le;
	fblock.line++;

	return ls;
}
