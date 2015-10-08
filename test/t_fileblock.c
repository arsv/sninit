#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

const char* testtxt = "t_fileblock.txt";

extern struct fileblock fblock;
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
	S(fblock.name, testtxt);
	A(fblock.len == testlen);
	A(fblock.ls == NULL);
	A(fblock.le == NULL);
	A(fblock.line == 0);

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
	A(fblock.len == shortlen);
	T(munmapfile());

	return 0;
}
