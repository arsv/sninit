#include <stdio.h>
#include "srv_.h"

const char* tag = "xftp";

int main(void)
{
	trapsig();
	say("starting");
	sleepx(10);
	say("normal exit");
	return 0;
}
