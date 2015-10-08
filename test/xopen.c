#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(void)
{
	int fd = open("xopen.c", O_RDONLY);
	return (fd <= 0 ? 1 + errno : 0);
}
