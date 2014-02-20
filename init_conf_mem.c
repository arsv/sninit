#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>

#include "init.h"
#include "init_conf.h"

int mremapblock(struct memblock* m, int size);

int mmapblock(struct memblock* m, int size)
{
	m->ptr = 0;
	if(m->addr) {
		if(m->len > size)
			return 0;
		else
			return mremapblock(m, size);
	} else {
		m->addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if(m->addr == MAP_FAILED)
			return -1;
		m->len = size;
		return 0;
	}
}

int mextendblock(struct memblock* m, int size, int blocksize)
{
	int ns = (blocksize > size ? blocksize : size);
	return mremapblock(m, m->len + ns);
}

int mremapblock(struct memblock* m, int size)
{
	void* np = mremap(m->addr, m->len, size, MREMAP_MAYMOVE);
	if(np == MAP_FAILED)
		return -1;

	m->addr = np;
	m->len = size;
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

/* copy s to m (adjusting ptr accordingly) and return offset at
   which s was placed in m */
int addstring(struct memblock* m, const char* string)
{
	const char* p;
	int r = m->ptr;
	for(p = string; *p; p++)
		*((char*)(m->addr + m->ptr++)) = *p;
	*((char*)(m->addr + m->ptr++)) = '\0';
	return r;
}

/* (m, char* a, char* b, char* c, ...) */
/* Make [ a, b, c, ... ] into an argv-style structure */
int addstrargarray(struct memblock* m, ...)
{
	va_list ap;
	int argc = 0;
	char* argi;
	int argl = 0;

	/* see how much space do we need */
	va_start(ap, m);
	while((argi = va_arg(ap, char*)))
		argc++, argl += strlen(argi);
	va_end(ap);

	/* allocate the space */
	if(mextendblock(m, (argc+1)*sizeof(char*) + argc + argl, IRALLOC))
		return -1;

	char** pa = m->addr + m->ptr; int pi = 0;
	/* skip over the pointers array */
	m->ptr += (argc+1)*sizeof(char*);
	/* copy strings */
	va_start(ap, m);
	while((argi = va_arg(ap, char*)) && pi++ < argc)
		*(pa++) = NULL + addstring(m, argi);
	/* terminate pointer array */
	while(pi++ <= argc)
		*(pa++) = NULL;

	return 0;
}

/* Treating str as n concatenated 0-terminated lines, append
   argv- or envp-like structure to m.
   The pointers array is always NULL-terminated.
   See prepargv() for how an array like this is formed. */
int addstringarray(struct memblock* m, int n, const char* str, const char* end)
{
	const char* p;

	if(mextendblock(m, (n+1)*sizeof(char*) + (end - str), IRALLOC))
		return -1;

	char** pa = m->addr + m->ptr;
	/* skip over pointers array */
	m->ptr += (n+1)*sizeof(char*);
	/* the first element is always there — it's the string itself */
	*pa++ = NULL + m->ptr;
	int pi = 1;
	/* copy the rest, char by char, advancing pa when necessary */
	for(p = str; p <= end; p++) {
		if((*((char*)(m->addr + m->ptr++)) = *p))
			continue;
		if(pi++ < n)
			*pa++ = NULL + m->ptr;
		else
			break;
	}; *pa = NULL;
	
	return 0;
}

/* Add envp-like array for $list to $m. $list is assumed to lie in $m. */
int addstringptrs(struct memblock* m, struct stringlist* list)
{
	int off;
	int ret = m->ptr;
	char** a;

	/* check how much space is needed, and allocate it */
	if(mextendblock(m, (list->count+1)*sizeof(char*), IRALLOC))
		return -1;

	/* "allocate" the structure */
	a = (char**)(m->addr + m->ptr);
	m->ptr += (list->count+1)*sizeof(char*);

	/* set up offsets; repoiting will happen later */
	for(off = list->head; off; off = blockptr(m, off, struct stringnode*)->next)
		*(a++) = NULL + off + offsetof(struct stringnode, str);
	*a = NULL; /* terminating pointer */

	return ret;
}

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

	for(le = ls; le < end && *le != '\n'; le++); *le = '\0';

	f->ls = ls;
	f->le = le;

	return 1;
}
