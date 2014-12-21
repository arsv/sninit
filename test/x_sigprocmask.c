#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include "test.h"

long signalled;

void sighandler(int sig)
{
	signalled |= (1 << sig);
}

int main(void)
{
	pid_t self = getpid();
	sigset_t defsigset;
	struct sigaction sa = {
		.sa_handler = sighandler,
		.sa_flags = SA_RESTART,
	};

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGCHLD);
	sigaddset(&sa.sa_mask, SIGURG);

	A(self > 0);
	T(sigaction(SIGCHLD, &sa, NULL));
	T(sigaction(SIGURG, &sa, NULL));

	signalled = 0;
	T(kill(self, SIGCHLD));
	A(signalled == (1 << SIGCHLD));
	T(kill(self, SIGURG));
	A(signalled == ((1 << SIGCHLD) | (1 << SIGURG)));

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGCHLD);
	T(sigprocmask(SIG_BLOCK, &sa.sa_mask, &defsigset));

	signalled = 0;
	T(kill(self, SIGCHLD));
	A(signalled == 0);
	T(kill(self, SIGURG));
	A(signalled == (1 << SIGURG));

	return 0;
}
