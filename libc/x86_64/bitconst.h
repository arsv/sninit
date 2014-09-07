#ifndef BITCONST_H
#define BITCONST_H

/* open and fcntl */
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#define O_CREAT		   0100
#define O_EXCL		   0200
#define O_NOCTTY	   0400
#define O_TRUNC		  01000
#define O_APPEND	  02000
#define O_NONBLOCK	  04000
#define O_DIRECTORY	0200000
#define O_NOFOLLOW	0400000

/* socket stuff */
#define AF_UNIX		1

#define SOCK_STREAM	1
#define SOCK_DGRAM	2

#define SO_PASSCRED	16
#define SOL_SOCKET	1

#define SCM_RIGHTS	0x01	/* rw: access rights (array of int) */
#define SCM_CREDENTIALS	0x02	/* rw: struct ucred             */
#define SCM_CONNECT	0x03	/* rw: struct scm_connect       */

/* mmap stuff */
#define MREMAP_MAYMOVE	1UL
#define MREMAP_FIXED	2UL

#define PROT_READ	0x1		/* page can be read */
#define PROT_WRITE	0x2		/* page can be written */
#define PROT_EXEC	0x4		/* page can be executed */
#define PROT_NONE	0x0		/* page can not be accessed */

#define MAP_SHARED	0x01		/* Share changes */
#define MAP_PRIVATE	0x02		/* Changes are private */
#define MAP_ANONYMOUS	0x20		/* don't use a file */

#define MAP_FAILED      ((void *) -1)

#endif
