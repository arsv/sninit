#include <stdio.h>
#include "srv_.h"

const char* tag = "srv2";

int main(void)
{
	trapsig();
	say("starting");
	sleepx(1000);
	say("normal exit");
	return 0;
}
