#include <stdio.h>
#include <string.h>

#include "../init.h"
#include "../init_conf.h"
#include "test.h"

#define RLVL 4

extern int readinitdir(struct fileblock* bb, const char* dir, int defrlvl, int strict);

int seen;

struct test {
	char* basename;
	char* content;
} testfiles[] = {
	{ "plain", "" },
	{ "script", "script contents" },
	{ "symlink", "plain contents" },
	{ NULL, NULL }
};

int findentry(char* basename)
{
	struct test* p;

	for(p = testfiles; p->basename; p++)
		if(!strcmp(p->basename, basename))
			return (p - testfiles);

	return -1;
}

int parsesrvfile(struct fileblock* fb, char* basename)
{
	int idx = findentry(basename);
	char* contents = (!strcmp(basename, "script") ? "script contents\n" : "plain contents\n");

	if(idx < 0) {
		printf("FAIL file %s should be skipped\n", basename);
	} else if(seen & (1 << idx)) {
		printf("FAIL file %s read twice (?!)\n", basename);
	} else {
		seen |= (1 << idx);
		printf("OK file %s\n", basename);
	}

	if(strcmp(fb->buf, contents))
		printf("FAIL file %s bad contents\n", basename);

	if(fb->rlvl != RLVL)
		printf("FAIL rlvl = %i\n", fb->rlvl);

	return 0;
}

int addinitrec(struct fileblock* fb, char* name, char* runlvl, char* flags, char* cmd, int exe)
{
	return 0;
}

int main(void)
{
	struct fileblock bb = {
		.name = "etc/inittab"
	};
	int ret;

	seen = 0;
	if((ret = readinitdir(&bb, "initdir", RLVL, 0)))
		printf("FAIL ret=%i\n", ret);

	if(seen != (1 << 0 | 1 << 1 | 1 << 2))
		printf("FAIL seen mask = 0x%02X\n", seen);
	else
		printf("OK seen mask\n");

	return 0;
}
