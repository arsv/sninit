#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

int currlevel;
struct config* cfg;

extern struct nblock newblock;
extern int mmapblock(int len);
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
NOCALL(readinittab);
NOCALL(readinitdir);
NOCALL(setrunflags);

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
	int hdrsize = sizeof(struct config) + sizeof(struct scratch);
	T(mmapblock(hdrsize));

	scratch_all(test_env);

	finishinittab();
	rewirepointers();

	B(newblock, NCF->env);
	check_all("env", NCF->env, test_env);

	return 0;
}
