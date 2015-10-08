#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

#define R0 (1<<0)
#define R1 (1<<1)
#define R2 (1<<2)
#define R3 (1<<3)
#define R01 (R0 | R1)

int currlevel = R2;
int state = 0;
struct initrec** inittab = NULL;

struct initrec c0 = { .name = "c0", .flags = C_ONCE, .rlvl = R1,  .pid = 0 };
struct initrec c1 = { .name = "c1", .flags = C_ONCE, .rlvl = R2,  .pid = -1 };
struct initrec c2 = { .name = "",   .flags = C_ONCE, .rlvl = R2,  .pid = -1 };
struct initrec c3 = { .name = "c3", .flags = 0,      .rlvl = R2,  .pid = 4 };
struct initrec c4 = { .name = "c4", .flags = 0,      .rlvl = R2,  .pid = 5 };

struct initrec n0 = { .name = "c0", .flags = C_ONCE, .rlvl = R1,  .pid = 0 };
struct initrec n1 = { .name = "c1", .flags = C_ONCE, .rlvl = R2,  .pid = -1 };
struct initrec n2 = { .name = "",   .flags = C_ONCE, .rlvl = R2,  .pid = 0 };
struct initrec n3 = { .name = "c3", .flags = 0,      .rlvl = R2,  .pid = 0 };
struct initrec n5 = { .name = "c5", .flags = 0,      .rlvl = R2,  .pid = 0 };

struct initrec* cfg_inittab[] = { &c0, &c1, &c2, &c3, &c4, NULL };
struct initrec* ncf_inittab[] = { &n0, &n1, &n2, &n3, &n5, NULL };

struct config cfg_struct = { .inittab = cfg_inittab };
struct config ncf_struct = { .inittab = ncf_inittab };

struct config* cfg = &cfg_struct;
struct config* ncf = &ncf_struct;

struct nblock newblock = { .addr = &ncf_struct, .len = 0, .ptr = 0 };

extern void transferpids(void);

NOCALL(readinittab);
NOCALL(readinitdir);
NOCALL(addptrsarray);
NOCALL(addstruct);
int levelmatch(struct initrec* p, int level) { return (p->rlvl & level); };

int main(void)
{
	transferpids();

	A(n0.pid == 0);		/* unrelated C_ONCE */
	A(n1.pid == -1);	/* effectively transferred from the old entry */
	A(n2.pid == -1);	/* current RL rule */
	A(n3.pid == 4);		/* named service has been transferred */
	A(n5.pid == 0);		/* new service, no pid change */

	return 0;
}
