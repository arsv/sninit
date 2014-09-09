#ifndef BITS_DIRENT_H
#define BITS_DIRENT_H

#include <bits/types.h>

#define DT_UNKNOWN	0
#define DT_FIFO		1
#define DT_CHR		2
#define DT_DIR		4
#define DT_BLK		6
#define DT_REG		8
#define DT_LNK		10
#define DT_SOCK		12
#define DT_WHT		14

struct dirent64 {
  uint64_t	d_ino;
  int64_t	d_off;
  uint16_t	d_reclen;
  unsigned char	d_type;
  char		d_name[256];
};

#endif
