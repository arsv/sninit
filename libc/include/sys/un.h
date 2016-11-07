#ifndef UN_H
#define UN_H

#include <sys/cdefs.h>
#include <bits/socket.h>

#define UNIX_PATH_MAX	108

struct sockaddr_un {
  sa_family_t sun_family;	/* AF_UNIX */
  char sun_path[UNIX_PATH_MAX];	/* pathname */
};

#endif
