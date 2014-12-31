#define _GNU_SOURCE
#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

struct memblock newblock;	/* to keep the linker happy */
struct memblock scratchblock;

int mextendblock() { return -1; }
int addstrargarray() { return -1; }
int checkdupname() { return -1; }
int addstringarray() { return -1; }
int scratchptr() { return -1; }
int addstring() { return -1; }

extern int prepargv(char* str, char** end);

void inplacetr(char* str, const char* from, const char* to)
{
	char* p;
	char* r;
	int off;
	int l = strlen(to);

	for(p = str; *p; p++) {
		if((r = strchr(from, *p)) == NULL)
			continue;
		off = r - from;
		if(off < 0 || off > l)
			continue;
		*p = to[off];
	}
}

void dumpstr(const char* str, int len, char* pref)
{
	int i;
	if(pref) printf("%s: ", pref);
	for(i = 0; i < len; i++)
		if(str[i] >= 0x20 && str[i] < 0x7F)
			printf("%c", str[i]);
		else switch(str[i]) {
			case '\0':
				printf("^"); break;
			default:
				printf("\\%02X", ((const unsigned char*)str)[i]);
		}
	printf("\n");

}

void test_prepargv(char* c_str, char* c_tst, int srcc, const char* mask, const char* repl)
{
	char* src = heapdup(c_str);
	char* tst = heapdup(c_tst);
	int len = strlen(src);
	int tstc;

	char* send = src + strlen(src);
	char* tend;

	if(mask && repl) {
		inplacetr(src, mask, repl);
		inplacetr(tst, mask, repl);
	}

	tstc = prepargv(src, &tend);
	A(tstc == srcc);
	A((tend - src) >= 0 && (tend - src) <= (send - src));
	T(memcmp(src, tst, len));
	if(tstc != srcc)
		printf("tstc=%i srcc=%i\n", tstc, srcc);
	if(memcmp(src, tst, len)) {
		dumpstr(src, len, "got");
		dumpstr(tst, len, "exp");
	}
}

int main(void)
{
	/* NOTE: inplacetr will be applied to all strings here */

	/* general parsing */
	test_prepargv(
		"/sbin/foo -a 30 -b 'foo bar' -c some! thing^",
		"/sbin/foo^-a^30^-b^foo bar^-c^some thing^^^^",
		7, "'!$^", "\"\\\n\0");

	/* do we skip spaces properly? */
	test_prepargv(
		" echo   -n  'foo  bar' ^",
		"echo^-n^foo  bar^^^^^^^^",
		3, "'^", "\"\0");

	/* how about embedded newlines? */
	test_prepargv(
		"echo -n$'foo  bar'^",
		"echo^-n^foo  bar^^^",
		3, "'$^", "\"\n\0");

	return 0;
}
