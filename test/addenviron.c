#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

char* test_env[] = {
	"PATH=/bin",
	"LANG=C",
	NULL
};

#define SN(p) ((struct stringnode*) p)

int main(void)
{
	int hdrsize = sizeof(struct config) + sizeof(struct scratch);
	ZERO(mmapblock(hdrsize));

	ASSERT(addenviron("PATH=/bin") >= 0);
	ASSERT(addenviron("LANG=C") >= 0);

	finishinittab();
	rewirepointers();

	ASSERT(NCF->env != NULL);
	INBLOCK(newblock, NCF->env);
	
	STREQUALS(NCF->env[0], "PATH=/bin");
	STREQUALS(NCF->env[1], "LANG=C");
	ASSERT(NCF->env[2] == NULL);

	return 0;
}
