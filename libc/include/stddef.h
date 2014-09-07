#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void*)0)

#ifdef __GNUC__
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __SIZE_TYPE__ size_t;
#else
typedef signed long ptrdiff_t;
typedef unsigned long size_t;
#endif

typedef signed long ssize_t;		/* Used for a count of bytes or an error indication. */

#define offsetof(type,member) ((size_t) &((type*)0)->member)

#endif
