#include <string.h>
#include <stdlib.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;
struct memblock scratchblock;

int addstringarray(int n, const char* str, const char* end);
extern int mmapblock(struct memblock* b, int len);
void munmapblock(struct memblock* m);

void check(char* tt, void** ep, char** test)
{
	char** p; int i;
	Bc(newblock, ep, "%s is valid", tt);
	Ac(ep != NULL, "%s is not null", tt);
	if(!ep) return;
	for(p = test, i = 0; *p; p++, i++)
		Ac((ep[i] - NULL >= 0) && ((ep[i] - NULL) <= newblock.len), "%s[%i] is valid", tt, i);

	for(p = test, i = 0; *p; p++, i++) {
		char* str = newblock.addr + (ep[i] - NULL);
		Ac(!strcmp(str, *p), "%s[%i] = %p \"%s\" expected \"%s\"", tt, i, ep[i], str, *p);
	}
	Ac(ep[i] == NULL, "%s[%i] = %p", tt, i, ep[i]);
}

int main(void)
{
	int off;
	if(mmapblock(&newblock, 100)) return -1;

	char* T1[] = { "foo", "bar", NULL };
	char S1[] = "foo\0bar\0";
	off = newblock.ptr;
	A(addstringarray(2, S1, S1 + 8) == 0);
	check("T1", newblock.addr + off, T1);

	char* T2[] = { "foo", "bar", NULL };
	char S2[] = "foo\0bar\0";
	off = newblock.ptr;
	A(addstringarray(2, S2, S2 + 8) == 0);
	check("T2", newblock.addr + off, T2);

	/* Single-string case */
	char* T3[] = { "foo", NULL };
	char S3[] = "foo\0";
	off = newblock.ptr;
	A(addstringarray(1, S3, S3 + 4) == 0);
	check("T3", newblock.addr + off, T3);

	/* Empty initial string */
	char* T4[] = { "", "bar", NULL };
	char S4[] = "\0bar\0";
	off = newblock.ptr;
	A(addstringarray(2, S4, S4 + 5) == 0);
	check("T4", newblock.addr + off, T4);

	return 0;
}
