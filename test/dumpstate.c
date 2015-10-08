#include "../config.h"
#include "../init.h"
#include "_test.h"

int currlevel = (1<<2);
int nextlevel = (1<<3);

struct initrec rec1 = { .name = "first",        .pid = 123,    .flags = 0,      };
struct initrec rec2 = { .name = "longrec2",     .pid = 1245,   .flags = 0,      };
struct initrec rec3 = { .name = "longhidden",   .pid = 123456, .flags = C_ONCE  };
struct initrec rec4 = { .name = "runonce",      .pid = 4,      .flags = C_ONCE  };
struct initrec rec5 = { .name = "last",         .pid = 2,      .flags = 0,      };

struct initrec* testinittab[] = { NULL, &rec1, &rec2, &rec3, &rec4, &rec5, NULL };

struct config testconfig = {
	.initnum = 5,
	.inittab = testinittab + 1,
	.env = NULL
};
struct config* cfg = &testconfig;

/* warn() logging -- there is only one warn() call */

extern int vsnprintf(char* buf, size_t len, const char* fmt, va_list ap);

#define BUFLEN 100
char warnbuf[BUFLEN];

int warn(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(warnbuf, BUFLEN, fmt, ap);
	va_end(ap);
	return 0;
}

/* dumprec() logging */

#define RECLEN 20
struct reclog {
	char name[NAMELEN];
	int nw;
	int pw;
} reclog[RECLEN];

int recptr = 0;

void dumprec(struct initrec* rec, int nw, int pw)
{
	if(recptr >= RECLEN) return;

	strcpy(reclog[recptr].name, rec->name);
	reclog[recptr].nw = nw;
	reclog[recptr].pw = pw;

	recptr++;
}

/* Simple level matching -- there's no submasks and no C_INVERT code here,
   and bringing init_pass.c in complicates linking a lot. */

int shouldbeshown(struct initrec* p)
{
	return !(p->flags & C_ONCE);
}

/* Finally the test code. There is little point in testing branches within
   dumpstate() as they are trivial. Instead, the point is to check dumprec
   call sequence for a given set of initrecs. */

extern void dumpstate(void);

#define Eqi(val, exp) Eq(val, exp, "%i")

#define TEST(i, tnm, tnw, tpw) { \
	S(reclog[i].name, tnm); \
	A(reclog[i].nw == tnw); \
	A(reclog[i].pw == tpw); \
}

int main(void)
{
	dumpstate();

	S(warnbuf, "#Switching 2 to 3");
	Eqi(recptr, 3);
	TEST(0, "first",    8, 4);
	TEST(1, "longrec2", 8, 4);
	TEST(2, "last",     8, 4);

	return 0;
}

NOCALL(levelmatch);
