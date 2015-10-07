#ifndef SOCKET_H
#define SOCKET_H

#include <sys/cdefs.h>
#include <sys/types.h>

#include <bits/socket.h>

struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};

struct ucred {
	pid_t pid;
	uid_t uid;
	gid_t gid;
};

int socket(int domain, int type, int protocol);
int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
int listen(int s, int backlog);
int shutdown(int s, int how);

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);

#endif
