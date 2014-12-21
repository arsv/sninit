#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "test.h"

/* Quick check to see whether O_NONBLOCK actually works.
   Control socket that is expected to be nonblocking but
   happens to be blocking wreaks init pretty badly. */

int main(void)
{
	int sfd;

	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = "\0x-accept-nonblock"
	};
	struct sockaddr peer;
	size_t plen = sizeof(peer);

	sfd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	A(sfd > 0);
	T(bind(sfd, (struct sockaddr*)&addr, sizeof(addr)));

	A(accept(sfd, &peer, &plen) < 0);

	return 0;
}
