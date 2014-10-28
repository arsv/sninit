#ifndef SYS_STAT_H
#define SYS_STAT_H

#include <bits/stat.h>

int stat(const char *pathname, struct stat *buf);
int fstat(int fd, struct stat *buf);

#endif
