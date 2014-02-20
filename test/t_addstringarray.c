#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock B;

int addstringarray(struct memblock* m, int n, const char* str, const char* end);
extern int mmapblock(struct memblock* b, int len);
void munmapblock(struct memblock* m);

void check(char* tt, void** ep, char** test)
{
	char** p; int i;
	Bc(B, ep, "%s is valid", tt);
	Ac(ep != NULL, "%s is not null", tt);
	if(!ep) return;
	for(p = test, i = 0; *p; p++, i++)
		Ac((ep[i] - NULL >= 0) && ((ep[i] - NULL) <= B.len), "%s[%i] is valid", tt, i);
	Ac(ep[i] == NULL, "%s[%i] = NULL", tt, i);

	for(p = test, i = 0; *p; p++, i++) {
		char* str = B.addr + (ep[i] - NULL);
		Ac(!strcmp(str, *p), "%s[%i] = \"%s\"", tt, i, str);
	}
}

int main(void)
{
	if(mmapblock(&B, 10)) return -1;
	char* T1[] = { "foo", "bar", NULL };
	char S1[] = "foo\0bar\0";
	A(addstringarray(&B, 2, S1, S1 + 8) == 0);
	check("T1", B.addr, T1);
	munmapblock(&B);

	return 0;
}
