#ifndef BITS_FCNTL_H
#define BITS_FCNTL_H

/* These are pretty much standard */
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02

/* These are NOT! */
#define O_CREAT         0x0100	/* not fcntl */
#define O_CREAT         0x0100	/* not fcntl */
#define O_TRUNC		0x0200	/* not fcntl */
#define O_EXCL		0x0400	/* not fcntl */
#define O_NOCTTY	0x0800	/* not fcntl */
#define FASYNC		0x1000	/* fcntl, for BSD compatibility */
#define O_LARGEFILE	0x2000	/* allow large file opens - currently ignored */
#define O_DIRECT	0x8000	/* direct disk access hint - currently ignored */
#define O_DIRECTORY	0x10000	/* must be a directory */
#define O_NOFOLLOW	0x20000	/* don't follow links */
#define O_NOATIME	0x40000
#define O_CLOEXEC	0x80000

#endif
