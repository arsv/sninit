#ifndef SYS_MMAN_H
#define SYS_MMAN_H

#include <bits/mman.h>
#include <bits/types.h>

void *mmap (void *addr, size_t len, int prot, int flags, int fd, off_t offset);
void *mremap (void *addr, size_t old_len, size_t new_len, int flags, ...);
int munmap (void *addr, size_t len);

#endif
