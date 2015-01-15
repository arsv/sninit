#ifndef BITS_SOCKET_H
#define BITS_SOCKET_H

#define AF_UNIX		1

#define SOCK_STREAM		1
#define SOCK_DGRAM		2
#define SOCK_NONBLOCK       04000
#define SOCK_CLOEXEC     02000000

#define SO_PASSCRED	16
#define SOL_SOCKET	1

#define SCM_RIGHTS	0x01	/* rw: access rights (array of int) */
#define SCM_CREDENTIALS	0x02	/* rw: struct ucred             */
#define SCM_CONNECT	0x03	/* rw: struct scm_connect       */

#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

#endif
