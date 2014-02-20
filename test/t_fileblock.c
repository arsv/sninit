#include <unistd.h>
#include <sys/stat.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

const char* testtxt = "t_fileblock.txt";

int mmapfile(struct fileblock* f, int maxlen);
int munmapfile(struct fileblock* f);
int nextline(struct fileblock* f);

int filelen(const char* file)
{
	struct stat st;
	if(stat(file, &st))
		return -1;
	return st.st_size;
}

int main(void)
{
	struct fileblock fb = { .name = testtxt };
	int testlen = filelen(testtxt);

	T(mmapfile(&fb, 1024));
	S(fb.name, testtxt);
	A(fb.len == testlen);
	A(fb.ls == NULL);
	A(fb.le == NULL);
	A(fb.line == 0);

	A(nextline(&fb) > 0);
	S(fb.ls, "line 1");
	A(nextline(&fb) > 0);
	S(fb.ls, "line 2");
	A(nextline(&fb) > 0);
	S(fb.ls, "");
	A(nextline(&fb) > 0);
	S(fb.ls, "somewhat longer line 4");
	A(nextline(&fb) == 0);

	T(munmapfile(&fb));

	int shortlen = testlen / 2;
	A(shortlen > 0);
	A(mmapfile(&fb, -shortlen) < 0);
	A(mmapfile(&fb,  shortlen) == 0);
	A(fb.len == shortlen);
	T(munmapfile(&fb));

	return 0;
}
