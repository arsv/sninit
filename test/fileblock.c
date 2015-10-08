#include <unistd.h>
#include <sys/stat.h>
#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

const char* testtxt = "fileblock.txt";

extern struct fblock fileblock;
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
	S(fileblock.name, testtxt);
	A(fileblock.len == testlen);
	A(fileblock.ls == NULL);
	A(fileblock.le == NULL);
	A(fileblock.line == 0);

	A((s = nextline()));
	S(s, "line 1");
	A((s = nextline()));
	S(s, "line 2");
	A((s = nextline()));
	S(s, "");
	A((s = nextline()));
	S(s, "somewhat longer line 4");
	A((s = nextline()) == NULL);

	T(munmapfile());

	int shortlen = testlen / 2;
	A(shortlen > 0);
	A(mmapfile(testtxt, -shortlen) < 0);
	A(mmapfile(testtxt,  shortlen) == 0);
	A(fileblock.len == shortlen);
	T(munmapfile());

	return 0;
}
