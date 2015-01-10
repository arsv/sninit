#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;
extern int memblockalign;

int addstrargarray(const char* argv[]);
extern int mmapblock(struct memblock* b, int len);
void munmapblock(struct memblock* m);

void check(char* tt, void** ep, const char* test[])
{
	const char** p; int i;
	Bc(newblock, ep, "%s is valid", tt);
	Ac(ep != NULL, "%s is not null", tt);
	if(!ep) return;
	for(p = test, i = 0; *p; p++, i++)
		Ac((ep[i] - NULL >= 0) && (ep[i] - NULL <= newblock.len), "%s[%i] is valid", tt, i);

	for(p = test, i = 0; *p; p++, i++) {
		char* str = newblock.addr + (ep[i] - NULL);
		Ac(!strcmp(str, *p), "%s[%i] = \"%s\"", tt, i, str);
	}
	Ac(ep[i] == NULL, "%s[%i] = NULL", tt, i);
}

int main(void)
{
	int off;

	memblockalign = 1;
	if(mmapblock(&newblock, 10)) return -1;

	const char* T1[] = { "foo", "bar", NULL };
	off = newblock.ptr;
	A(addstrargarray(T1) == 0);
	check("T1", newblock.addr + off, T1);

	const char* T2[] = { "foo", NULL };
	off = newblock.ptr;
	A(addstrargarray(T2) == 0);
	check("T2", newblock.addr + off, T2);

	return 0;
}
