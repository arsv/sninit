#ifndef TYPES_H
#define TYPES_H

#include <inttypes.h>
#include <stddef.h>

typedef uint32_t socklen_t;
typedef uint16_t sa_family_t;


typedef int32_t pid_t;			/* Used for process IDs and process group IDs. */
#if defined(__alpha__) || defined(__ia64__) || defined(__sparc64__) || defined(__s390x__)
    typedef uint32_t gid_t;		/* Used for group IDs. */
    typedef uint32_t uid_t;		/* Used for user IDs. */
#elif defined(__arm__) || defined(__i386__) || defined(__sparc__) || defined(__s390__) /* make sure __s390x__ hits before __s390__ */
    typedef uint16_t gid_t;
    typedef uint16_t uid_t;
#elif defined(__hppa__)
    typedef uint32_t gid_t;
    typedef uint32_t uid_t;
#elif defined(__mips__)
    typedef int32_t gid_t;
    typedef int32_t uid_t;
#elif defined(__powerpc__) && !defined(__powerpc64__)
    typedef uint32_t gid_t;
    typedef uint32_t uid_t;
#elif defined(__powerpc64__) || defined(__x86_64__)
    typedef uint32_t gid_t;
    typedef uint32_t uid_t;
#endif

#if defined(__x86_64__) && defined(__ILP32__)
typedef signed long long time_t;
#else
typedef signed long time_t;		/* Used for time in seconds. */
#endif

typedef signed long off_t;             /* Used for file sizes. */

struct dirent64 {
  uint64_t	d_ino;
  int64_t	d_off;
  uint16_t	d_reclen;
  unsigned char	d_type;
  char		d_name[256];
};

#endif
