#ifndef BITS_SOCKET_H
#define BITS_SOCKET_H

#define AF_UNIX		1

#define SOCK_DGRAM	(1<<0)	/* MIPS! */
#define SOCK_STREAM	(1<<1)	/* (all other arches have it the other way around) */
#define SOCK_NONBLOCK	(1<<7)	/* MIPS! */
#define SOCK_CLOEXEC	(1<<19)	/* common value */

#define SO_PEERCRED	18	/* MIPS! */
#define SOL_SOCKET	0xffff	/* MIPS! */

#define SHUT_RD		0
#define SHUT_WR		1
#define SHUT_RDWR	2

#endif
