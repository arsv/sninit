#ifndef UNISTD_H
#define UNISTD_H

#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

pid_t getpid(void);
gid_t getgid(void);
uid_t getuid(void);
pid_t setsid(void);

int setresuid(uid_t ruid, uid_t euid, uid_t suid);
int setresgid(gid_t rgid, gid_t egid, gid_t sgid);
int setpgid(pid_t pid, pid_t pgid);

void _exit(int status) __attribute__((__noreturn__));

int open(const char* pathname,int flags, ...);
int close(int fd);
ssize_t write(int fd,const void* buf,size_t len);
ssize_t read(int fd,void* buf,size_t len);
int fcntl(int fd, int cmd, ...);

pid_t fork(void);
pid_t vfork(void);
int execve(const char *filename, char *const argv [], char *const envp[]);
int execvp(const char *file, char *const argv[]);

time_t time(time_t *t);

int dup2 (int oldfd,int newfd);

int chroot(const char *path);
int chdir(const char *path);

#endif
