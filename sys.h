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
