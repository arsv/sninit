#define _GNU_SOURCE
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include "init.h"
#include "init_conf.h"

/* Whenever possible, memory is mmaped in memblockalign increments
   to reduce the number of calls. To make the whole thing testable,
   int is used instead of placing IRALLOC in all relevant calls. */
int memblockalign = IRALLOC;

/* m*block routines work with anonymous mmaped areas defined by struct memblock.
   m*file routines are for mmaped files, struct fileblock. */

int mmapblock(struct memblock* m, int size)
{
	m->ptr = 0;

	int aligned = size;
	if(size % memblockalign)
		aligned += (memblockalign - size % memblockalign);

	if(m->addr) {
		/* This is a relatively unlikely case when a new reconfigure
		   request comes before newblock from the previous one
		   is moved over to cfgblock. In such a case, try to re-use
		   newblock without unmmaping it. Due to the way mextendblock
		   works, the old block must be large enough; if it is not,
		   it's a hard error. */
		if(m->len < size)
			return -1;
		memset(m->addr, 0, m->len);
	} else {
		m->addr = mmap(NULL, aligned, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if(m->addr == MAP_FAILED)
			return -1;
		m->len = aligned;
	}

	return 0;
}

int mextendblock(struct memblock* m, int size)
{
	if(m->len - m->ptr > size)
		return 0;
	
	if(size % memblockalign)
		size += (memblockalign - size % memblockalign);

	/* size is just an int, overflows are possible (but very unlikely) */
	if(size < 0 || m->len + size < 0)
		return -1;

	void* np = mremap(m->addr, m->len, m->len + size, MREMAP_MAYMOVE);
	if(np == MAP_FAILED)
		return -1;

	m->addr = np;
	m->len += size;
	return 0;
}

void munmapblock(struct memblock* m)
{
	if(m->addr) {
		munmap(m->addr, m->len);
		m->addr = 0;
		m->len = 0;
	}
};

/* Due to average inittab being about 1-2k, it's always read whole;
   for service files, only the head is mmaped.
   Also, init makes no distinction between mmap failure and open failure,
   both mean the new inittab won't be used */
/* maxlen > 0: maximum size to map; maxlen < 0: maximum file size to mmap whole */
/* the result is always 0-terminated */
int mmapfile(struct fileblock* f, int maxlen)
{
	struct stat st;

	int fd = open(f->name, O_RDONLY);
	if(fd < 0)
		retwarn(-1, "can't open %s: %m", f->name);

	if(fstat(fd, &st) < 0)
		gotowarn(out, "can't stat %s: %m", f->name);

	if(!S_ISREG(st.st_mode))
		gotowarn(out, "%s: not a regular file", f->name);
	if(maxlen < 0 && st.st_size > -maxlen)
		/* Note: because ints are used in lots of places,
		   it is a good idea to avoid loading anything that
		   exceeds 2^31 when compiled into newblock */
		gotowarn(out, "%s: file too large", f->name);

	int stm = st.st_size;
	if(maxlen > 0 && stm > maxlen) stm = maxlen;

	/* with one guard byte at the end, to hold \0 */
	f->len = stm;
	f->buf = mmap(NULL, stm + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if(f->buf == MAP_FAILED)
		gotowarn(out, "%s: mmap failed: %m", f->name);
	f->buf[stm] = '\0';

	f->ls = NULL;
	f->le = NULL;
	f->line = 0;

	close(fd);
	return 0;

out:	close(fd);
	return -1;
}

int munmapfile(struct fileblock* f)
{
	return munmap(f->buf, f->len);
}

int nextline(struct fileblock* f)
{
	char* le = f->le;
	char* ls = le ? le + 1 : f->buf;
	char* end = f->buf + f->len;

	if(ls >= end) return 0;

	for(le = ls; le < end && *le != '\n'; le++)
		; /* clang is full of hatred towards elegant concise expressions */
	*le = '\0';

	f->ls = ls;
	f->le = le;
	f->line++;

	return 1;
}
