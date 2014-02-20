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
extern int scratchstring(char listcode, char* string);
extern void initcfgblocks(void);
extern int finishenvp(void);
extern void rewireenvp(char*** envp);

int warnfd = 2;
int syslogfd = -1;

char* test_env[] = {
	"PATH=/bin",
	"LANG=C",
	NULL
};

char* test_dir[] = {
	"/etc/rc.1",
	"/etc/rc.3",
	NULL
};

#define SN(p) ((struct stringnode*) p)

void scratch_all(char t, char** test)
{
	char** p;
	for(p = test; *p; p++)
		Ac(scratchstring(t, *p) >= 0, "scratch(%c, \"%s\")", t, *p);
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
	newblock.ptr++;

	scratch_all('E', test_env);
	scratch_all('D', test_dir);

	finishenvp();
	rewireenvp(&(NCF->env));
	rewireenvp(&(NCF->dir));

	check_all("env", NCF->env, test_env);
	check_all("dir", NCF->dir, test_dir);
	B(newblock, NCF->env);

	return 0;
}
