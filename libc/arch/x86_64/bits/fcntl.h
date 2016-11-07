#ifndef BITS_FCNTL_H
#define BITS_FCNTL_H

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

#define F_DUPFD  0
#define F_GETFD  1
#define F_SETFD  2

#define AT_FDCWD -100

#endif
