#ifndef BITS_SOCKET_H
#define BITS_SOCKET_H

#define AF_UNIX		1

#define SOCK_DGRAM	1	/* MIPS! */
#define SOCK_STREAM	2	/* (all other arches have it the other way around) */
#define SOCK_NONBLOCK  0200	/* MIPS! */
#define SOCK_CLOEXEC 02000000	/* common value */

#define SO_PASSCRED	17	/* MIPS-specific */
#define SOL_SOCKET	0xffff	/* MIPS-specific */

#define SCM_RIGHTS	0x01	/* rw: access rights (array of int) */
#define SCM_CREDENTIALS	0x02	/* rw: struct ucred             */
#define SCM_CONNECT	0x03	/* rw: struct scm_connect       */

#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

#endif
