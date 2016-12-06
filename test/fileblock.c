#include <unistd.h>
#include <sys/stat.h>
#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

const char* testtxt = "fileblock.txt";

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

	ZERO(mmapfile(testtxt, 1024));
	STREQUALS(fileblock.name, testtxt);
	ASSERT(fileblock.len == testlen);
	ASSERT(fileblock.ls == NULL);
	ASSERT(fileblock.le == NULL);
	ASSERT(fileblock.line == 0);

	ASSERT((s = nextline()));
	STREQUALS(s, "line 1");
	ASSERT((s = nextline()));
	STREQUALS(s, "line 2");
	ASSERT((s = nextline()));
	STREQUALS(s, "");
	ASSERT((s = nextline()));
	STREQUALS(s, "somewhat longer line 4");
	ASSERT((s = nextline()) == NULL);

	ZERO(munmapfile());

	int shortlen = testlen / 2;
	ASSERT(shortlen > 0);
	ASSERT(mmapfile(testtxt, -shortlen) < 0);
	ASSERT(mmapfile(testtxt,  shortlen) == 0);
	ASSERT(fileblock.len == shortlen);
	ZERO(munmapfile());

	return 0;
}
