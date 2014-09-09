#ifndef DIRENT_H
#define DIRENT_H

#include <bits/dirent.h>

int getdents64(int fd, struct dirent64 *dirp, unsigned int count);

#endif
