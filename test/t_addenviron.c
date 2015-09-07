#include <string.h>
#include "../init.h"
#include "../init_conf.h"
#include "test.h"

int state;
int currlevel;
struct config* cfg;
char* inittab = NULL;

extern struct memblock newblock;
extern int mmapblock(struct memblock* b, int len);
extern void initcfgblocks(void);
extern int finishinittab(void);
extern void rewirepointers();

extern int addenviron(const char* def);

int warnfd = 2;
int syslogfd = -1;

char* test_env[] = {
	"PATH=/bin",
	"LANG=C",
	NULL
};

NOCALL(levelmatch);

#define SN(p) ((struct stringnode*) p)

void scratch_all(char** test)
{
	char** p;
	for(p = test; *p; p++)
		Ac(addenviron(*p) >= 0, "addenviron(\"%s\")", *p);
}

void check_all(char* tt, char** ep, char** test)
{
	char** p; int i;
	Bc(newblock, ep, "%s is valid", tt);
	Ac(ep != NULL, "%s is not null", tt);
	if(!ep) return;
	for(p = test, i = 0; *p; p++, i++)
		Bc(newblock, ep[i], "%s[%i] is valid", tt, i);
	Ac(ep[i] == NULL, "%s[%i] = NULL", tt, i);

	for(p = test, i = 0; *p; p++, i++)
		Ac(!strcmp(ep[i], *p), "%s[%i] = \"%s\"", tt, i, ep[i]);
}

int main(void)
{
	T(mmapblock(&newblock, 1024));
	initcfgblocks();

	scratch_all(test_env);

	finishinittab();
	rewirepointers();

	check_all("env", NCF->env, test_env);
	B(newblock, NCF->env);

	return 0;
}
