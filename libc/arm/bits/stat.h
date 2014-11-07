#ifndef BITS_STAT_H
#define BITS_STAT_H

#include <bits/types.h>

#define S_IFMT	       00170000
#define S_IFSOCK	0140000
#define S_IFLNK		0120000
#define S_IFREG		0100000
#define S_IFBLK		0060000
#define S_IFDIR		0040000
#define S_IFCHR		0020000
#define S_IFIFO		0010000
#define S_ISUID		0004000
#define S_ISGID		0002000
#define S_ISVTX		0001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

struct stat {
	uint32_t	st_dev;
	unsigned long	st_ino;
	uint16_t	st_mode;
	uint16_t	st_nlink;
	uint16_t	st_uid;
	uint16_t	st_gid;
	uint32_t	st_rdev;
	unsigned long	st_size;
	unsigned long	st_blksize;
	unsigned long	st_blocks;
	time_t		st_atime;
	unsigned long	st_atime_nsec;
	time_t		st_mtime;
	unsigned long	st_mtime_nsec;
	time_t		st_ctime;
	unsigned long	st_ctime_nsec;
	unsigned long	__unused4;
	unsigned long	__unused5;
};

#endif
