/* dietlibc lacks this definition */
#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK 00004000
#endif

/* glibc intentionally omits getdents */
#ifdef __GLIBC__
struct dirent64;
extern int getdents64(int fd, struct dirent64 *dirp, size_t count);
#endif
