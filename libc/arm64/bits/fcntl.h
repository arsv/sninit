#ifndef BITS_FCNTL_H
#define BITS_FCNTL_H

/* These are pretty much standard */
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02

/* These are NOT! */
#define O_CREAT		    0100	/* not fcntl */
#define O_EXCL		    0200	/* not fcntl */
#define O_NOCTTY	    0400	/* not fcntl */
#define O_TRUNC		   01000	/* not fcntl */
#define O_APPEND	   02000
#define O_NONBLOCK	   04000
#define O_DIRECTORY	  040000	/* must be a directory */
#define O_NOFOLLOW	 0100000	/* don't follow links */
#define O_DIRECT	 0200000	/* direct disk access hint - currently ignored */
#define O_LARGEFILE	 0400000
#define O_NOATIME	01000000
#define O_CLOEXEC	02000000

#endif
