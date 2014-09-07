#ifndef UNISTD_H
#define UNISTD_H

#include <sys/cdefs.h>
#include <sys/types.h>
#include <unitypes.h>
#include <uniconst.h>

pid_t getpid(void);
gid_t getgid(void);

uid_t geteuid(void);
gid_t getegid(void);

pid_t setsid (void);

void _exit(int status) __attribute__((__noreturn__));

int open(const char* pathname,int flags, ...);
int openat(int dirfd, const char *pathname, int flags, ...);
ssize_t write(int fd,const void* buf,size_t len);
ssize_t read(int fd,void* buf,size_t len);
int getdents64(int fd, struct dirent64 *dirp, unsigned int count);
int close(int fd);

pid_t fork(void);
int execve(const char *filename, char *const argv [], char *const envp[]);

void *mmap (void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void *mremap (void *addr, size_t old_len, size_t new_len, unsigned long may_move);
int munmap (void *addr, size_t len);

time_t time(time_t *t);

int dup2 (int oldfd,int newfd);
int fstat(int fd, struct stat *buf);

#endif
