#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#ifndef __dietlibc__
#include <sys/syscall.h>
#else
/* XXX: syscall() is declared as "int" in dietlibc; it must be at least long int for x64 */
#include <asm/unistd.h>
long syscall(int number, ...);
#endif

#include "../init_conf.h"
#include "test.h"

extern int memblockalign;

extern int mmapblock(struct memblock* m, int size);
extern int mextendblock(struct memblock* m, int size);
extern int mremapblock(struct memblock* m, int size);
extern void munmapblock(struct memblock* m);

off_t BASE = 0x00;

unsigned char bytesign(int off)
{
	return off & 0xFF;
}

#ifdef __dietlibc__
void* mremap(void* old_addr, size_t old_size, size_t new_size, unsigned long flags)
#else
void* mremap(void* old_addr, size_t old_size, size_t new_size, int flags, ...)
#endif
{
	/* Libc sanity is assumed here:  mremap(...) === syscall(__NR_mremap, ...) */
	if(!BASE) {
		return (void*)syscall(__NR_mremap, old_addr, old_size, new_size, flags);
	} else {
		/* let's just assume BASE is page-aligned (4kB typically) */
		flags |= MREMAP_FIXED;
		return (void*)syscall(__NR_mremap, old_addr, old_size, new_size, flags, NULL + BASE);
	}
}

int can_write(struct memblock* m, int start, int len)
{
	int i;

	for(i = start; i < len; i++) {
		m->ptr = i;
		((unsigned char*)m->addr)[m->ptr] = bytesign(m->ptr);
	}

	return 0;
}

int can_read_back(struct memblock* m, int start, int len)
{
	int i;
	
	for(i = start; i < len; i++)
		if(((unsigned char*)m->addr)[i] != bytesign(i))
			return -1;

	return 0;
}

//void test_addstuff()
//{
//	struct memblock m;
//	struct teststring {
//		char* src;
//		char* tst;
//	} tests[] = {
//		{ "test123", NULL },
//		{ "some other string", NULL },
//		{ NULL }
//	}, *p;
//	int len;
//
//	/* just enough space for all strings */
//	for(len = 0, p = tests; p && p->src; p++)
//		len += strlen(p->src) + 1;
//
//	T(mmapblock(&m, len));
//
//	/* check if strings are copied properly */
//	for(p = tests; p && p->src; p++) {
//		A((p->tst = m.addr + addstring(&m, p->src)) != NULL);
//		T(strcmp(p->src, p->tst));
//	}
//	/* check all the string are readable (=no overwrites) */
//	for(p = tests; p && p->src; p++)
//		T(strcmp(p->src, p->tst));
//
//	munmapblock(&m);
//}

int main(void)
{
	struct memblock m;
	int len = 12345;
	int ext = 500137;

	int off;

	T(mmapblock(&m, len));
	A(m.ptr == 0);
	A(m.len >= len);
	T(can_write(&m, 0, len));
	T(can_read_back(&m, 0, len));

	off = m.ptr;
	len += ext;
	T(mextendblock(&m, ext));
	A(m.ptr == off);
	T(can_write(&m, m.ptr, len));
	T(can_read_back(&m, 0, len));
	m.ptr = 0;
	T(can_write(&m, 0, len));
	T(can_read_back(&m, 0, len));

	BASE = 0x75800000;	/* Beware this *may* happen to be signed! (MIPS) */
	off = m.ptr;
	len += ext;
	T(mextendblock(&m, ext));
	A(m.ptr == off);
	T(can_write(&m, m.ptr, len));
	T(can_read_back(&m, 0, len));
	m.ptr = 0;
	T(can_write(&m, 0, len));
	T(can_read_back(&m, 0, len));

	munmapblock(&m);

	return 0;
}
