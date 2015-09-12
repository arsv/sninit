#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
	pid_t pid;
	int status;
	char* cmd[] = { "./x_hello", NULL };
	char* env[] = { NULL };

	if((pid = vfork()) < 0) {
		return 13;
	} else if(pid) {
		return (waitpid(pid, &status, 0) < 0 ? 17 : 0);
	} else {
		execve(*cmd, cmd, env);
		_exit(5);
	}

	return 0;
}
