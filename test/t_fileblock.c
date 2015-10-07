#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

const char* testtxt = "t_fileblock.txt";

extern struct fileblock fb;
int mmapfile(const char* name, int maxlen);
int munmapfile(void);
char* nextline(void);

int filelen(const char* file)
{
	struct stat st;
	if(stat(file, &st))
		return -1;
	return st.st_size;
}

int main(void)
{
	int testlen = filelen(testtxt);
	char* s;

	T(mmapfile(testtxt, 1024));
	S(fb.name, testtxt);
	A(fb.len == testlen);
	A(fb.ls == NULL);
	A(fb.le == NULL);
	A(fb.line == 0);

	A((s = nextline()));
	S(fb.ls, "line 1");
	A((s = nextline()));
	S(fb.ls, "line 2");
	A((s = nextline()));
	S(fb.ls, "");
	A((s = nextline()));
	S(fb.ls, "somewhat longer line 4");
	A((s = nextline()) == NULL);

	T(munmapfile());

	int shortlen = testlen / 2;
	A(shortlen > 0);
	A(mmapfile(testtxt, -shortlen) < 0);
	A(mmapfile(testtxt,  shortlen) == 0);
	A(fb.len == shortlen);
	T(munmapfile());

	return 0;
}
