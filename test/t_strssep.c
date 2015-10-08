#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct newblock nblock;
NOCALL(extendblock);

extern char* strssep(char** str);

int main(void)
{
	char str[] = "Some  words \t go    here  ";
	char* ptr = str;
	char* res;

	res = strssep(&ptr); S(res, "Some");
	res = strssep(&ptr); S(res, "words");
	res = strssep(&ptr); S(res, "go");
	res = strssep(&ptr); S(res, "here");
	res = strssep(&ptr); S(res, NULL);
	A(ptr == NULL);
	res = strssep(&ptr); S(res, NULL);
	A(ptr == NULL);

	return 0;
}
