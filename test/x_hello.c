#include <unistd.h>

int main(int argc, char** argv)
{
	int l = 14;
	int r = write(1, "Hello, world!\n", l);

	if(r < 0)
		return -1;
	if(r != l)
		return 1 + r;
	else
		return 0;
}
