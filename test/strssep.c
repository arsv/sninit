#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

struct nblock newblock;

int main(void)
{
	char str[] = "Some  words \t go    here  ";
	char* ptr = str;
	char* res;

	res = strssep(&ptr); STREQUALS(res, "Some");
	res = strssep(&ptr); STREQUALS(res, "words");
	res = strssep(&ptr); STREQUALS(res, "go");
	res = strssep(&ptr); STREQUALS(res, "here");
	res = strssep(&ptr); STREQUALS(res, NULL);
	ASSERT(ptr == NULL);
	res = strssep(&ptr); STREQUALS(res, NULL);
	ASSERT(ptr == NULL);

	return 0;
}
