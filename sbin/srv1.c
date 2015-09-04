#include "srv_.h"

const char* tag = "srv1";

int main(void)
{
	trapsig();
	say("starting");
	sleepx(20);
	say("normal exit");
	return 0;
}
