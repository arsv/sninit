#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock B;

int addstrargarray(struct memblock* m, ...);
extern int mmapblock(struct memblock* b, int len);
void munmapblock(struct memblock* m);

void check(char* tt, void** ep, char** test)
{
	char** p; int i;
	Bc(B, ep, "%s is valid", tt);
	Ac(ep != NULL, "%s is not null", tt);
	if(!ep) return;
	for(p = test, i = 0; *p; p++, i++)
		Ac((ep[i] - NULL >= 0) && (ep[i] - NULL <= B.len), "%s[%i] is valid", tt, i);
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
	A(addstrargarray(&B, "foo", "bar", NULL) == 0);
	check("T1", B.addr, T1);
	munmapblock(&B);

	if(mmapblock(&B, 10)) return -1;
	char* T2[] = { "foo", NULL };
	A(addstrargarray(&B, "foo", NULL) == 0);
	check("T2", B.addr, T2);
	munmapblock(&B);

	return 0;
}
