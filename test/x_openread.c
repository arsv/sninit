#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "test.h"

int main(void)
{
	char* file = "x_openread.txt";
	char buf[100];
	int len;
	int wrt;

	int fd = open(file, O_RDONLY);
	len = read(fd, buf, 100);
	if(len < 0)
		die("Can't read %s: %m\n", file);

	wrt = write(1, buf, len);

	if(wrt < 0)
		die("Can't dump data to stdout: %m\n");

	return 0;
}
