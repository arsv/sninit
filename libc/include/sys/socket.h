#ifndef SOCKET_H
#define SOCKET_H

#include <sys/cdefs.h>
#include <sys/types.h>

#include <bitconst.h>

struct sockaddr {
  sa_family_t sa_family;
  char sa_data[14];
};

struct msghdr {
  void* msg_name;		/* Socket name */
  socklen_t msg_namelen;		/* Length of name */
  struct iovec* msg_iov;	/* Data blocks */
  size_t msg_iovlen;		/* Number of blocks */
  void* msg_control;		/* Per protocol magic (eg BSD file descriptor passing) */
  size_t msg_controllen;	/* Length of cmsg list */
  uint32_t msg_flags;
};

struct cmsghdr {
  size_t cmsg_len;	/* data byte count, including hdr */
  int32_t cmsg_level;	/* originating protocol */
  int32_t cmsg_type;	/* protocol-specific type */
};

struct iovec {
  void* iov_base;	/* BSD uses caddr_t (1003.1g requires void *) */
  size_t iov_len;	/* Must be size_t (1003.1g) */
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
int recv(int s, void *buf, size_t len, int flags);
int recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int recvmsg(int s, struct msghdr *msg, int flags);
int send(int s, const void *msg, size_t len, int flags);
int sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
int sendmsg(int s, const struct msghdr *msg, int flags);

int getpeername(int s, struct sockaddr *name, socklen_t *namelen);
int getsockname(int  s , struct sockaddr * name , socklen_t * namelen);

int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);

int listen(int s, int backlog);

#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2
int shutdown(int s, int how);


#define CMSG_ALIGN(len) ( ((len)+sizeof(long)-1) & ~(sizeof(long)-1) )
#define __CMSG_NXTHDR(ctl, len, cmsg) __cmsg_nxthdr((ctl),(len),(cmsg))
#define CMSG_NXTHDR(mhdr, cmsg) cmsg_nxthdr((mhdr), (cmsg))

static inline struct cmsghdr* __cmsg_nxthdr(void *__ctl, size_t __size, struct cmsghdr *__cmsg)
{
  struct cmsghdr * __ptr;
  __ptr = (struct cmsghdr*)(((unsigned char *) __cmsg) +  CMSG_ALIGN(__cmsg->cmsg_len));
  if ((unsigned long)((char*)(__ptr+1) - (char *) __ctl) > __size)
    return (struct cmsghdr *)0;
  return __ptr;
}

static inline struct cmsghdr* cmsg_nxthdr (struct msghdr *__msg, struct cmsghdr *__cmsg)
{
  return __cmsg_nxthdr(__msg->msg_control, __msg->msg_controllen, __cmsg);
}

#define CMSG_DATA(cmsg)	((void *)((char *)(cmsg) + CMSG_ALIGN(sizeof(struct cmsghdr))))
#define CMSG_SPACE(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(len))
#define CMSG_LEN(len) (CMSG_ALIGN(sizeof(struct cmsghdr)) + (len))

#define __CMSG_FIRSTHDR(ctl,len) ((len) >= sizeof(struct cmsghdr) ? \
				  (struct cmsghdr *)(ctl) : \
				  (struct cmsghdr *)NULL)
#define CMSG_FIRSTHDR(msg)	__CMSG_FIRSTHDR((msg)->msg_control, (msg)->msg_controllen)

#endif
