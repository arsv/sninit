#ifndef BITS_SOCKET_H
#define BITS_SOCKET_H

#define AF_UNIX		1

#define SOCK_STREAM	(1<<0)
#define SOCK_DGRAM	(1<<1)
#define SOCK_NONBLOCK	(1<<11)
#define SOCK_CLOEXEC	(1<<19)

#define SOL_SOCKET	1
#define SO_PEERCRED	17

#define SHUT_RD		0
#define SHUT_WR		1
#define SHUT_RDWR	2

#endif
