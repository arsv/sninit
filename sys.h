/* dietlibc lacks this definition */
#ifndef SOCK_NONBLOCK
#ifdef __mips__
#define SOCK_NONBLOCK 00000200
#else
#define SOCK_NONBLOCK 00004000
#endif
#endif

/* again, dietlibc */
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 02000000
#endif

/* glibc intentionally omits getdents */
#ifdef __GLIBC__
struct dirent64;
extern int getdents64(int fd, struct dirent64 *dirp, size_t count);
#endif

#ifdef __UCLIBC__
extern int getdents64(int fd, struct dirent64 *dirp, size_t count);
#endif

/* Bad bad hack to sort out struct dirent64 issue.
   Both libraries provide no getdents{,64} but do, however, define
   struct dirent64, sometimes, depending on the features requested
   and (in case of uClibc at least) on build-time library configuration.
   
   And since sys_dents.c uses __NR_getdents64 invariably, the structure
   must be defined, and must be named exactly dirent64.
 
   To make our lives more interesting, neither library gives a clear
   indication on whether struct dirent64 has been defined.
   The test below depends on _LARGEFILE64_SOURCE being defined
   if _GNU_SOURCE has been used, but only if the library supports largefile64. 
   See uclibc features.h */

#if defined(__GLIBC__) || defined(__UCLIBC__)
#ifndef _LARGEFILE64_SOURCE  /* this depends on #define _GNU_SOURCE in init_conf_dir.c! */
#include <stdint.h>
struct dirent64
{
	uint64_t d_ino;
	uint64_t d_off;
	unsigned short int d_reclen;
	unsigned char d_type;
	char d_name[256];
};
#endif
#endif
