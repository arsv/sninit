#include <stdio.h>
#include "srv_.h"

const char* tag = "srv3";

int main(void)
{
	trapsig();
	say("starting");
	sleepx(100);
	say("normal exit");
	return 0;
}
