#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOG(fmt, ...) \
	printf("# " fmt "\n", ## __VA_ARGS__);

/* test if success (zero) */
#define ZERO(a) {\
	int r = a;\
	if(r)\
		printf("%s:%i: FAIL(%i) %s\n", __FILE__, __LINE__, r, #a);\
	else\
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, #a);\
}

#define ASSERT(a) \
	if(a)\
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, #a);\
	else\
		printf("%s:%i: FAIL %s\n", __FILE__, __LINE__, #a);

#define Ac(a, fmt, ...) \
	if(a)\
		printf("%s:%i: OK " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);\
	else\
		printf("%s:%i: FAIL " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);

/* compare strings */
#define STREQUALS(a,b) {\
	const char* r = a;\
	if(b == NULL && r == NULL)\
		printf("%s:%i: OK %s = NULL\n", __FILE__, __LINE__, #a);\
	else if(b == NULL) \
		printf("%s:%i: FAIL %s = \"%s\" not NULL\n", __FILE__, __LINE__, #a, r);\
	else if(r == NULL) \
		printf("%s:%i: FAIL %s = NULL\n", __FILE__, __LINE__, #a);\
	else if(strcmp(r, b)) \
		printf("%s:%i: FAIL %s = \"%s\" not \"%s\"\n", __FILE__, __LINE__, #a, r, b);\
	else\
		printf("%s:%i: OK %s = \"%s\"\n", __FILE__, __LINE__, #a, b);\
}

/* check pointer p against block b */
#define INBLOCK(b, p) {\
	if(p && !((void*)p >= b.addr && (void*)p <= b.addr + b.len))\
		printf("%s:%i: FAIL %s=%p %s=[%p +%i]\n", __FILE__, __LINE__, #p, p, #b, b.addr, b.len);\
	else\
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, #p);\
}

/* check pointer p against block b */
#define INBLOCK_(b, p, fmt, ...) {\
	if(p && !((void*)p >= b.addr && (void*)p <= b.addr + b.len))\
		printf("%s:%i: FAIL " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);\
	else\
		printf("%s:%i: OK " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);\
}

#define EQUALS_(val, exp, fmt) {\
	if((val) == (exp)) \
		printf("%s:%i: OK %s == %s\n", __FILE__, __LINE__, #val, #exp);\
	else \
		printf("%s:%i: FAIL %s = " fmt " != %s\n", __FILE__, __LINE__, #val, val, #exp);\
}

#define INTEQUALS(val, exp) \
	EQUALS_(val, exp, "%i")
#define HEXEQUALS(val, exp) \
	EQUALS_(val, exp, "%04X")

char* heapdup(const char* s);
char* heapdupnull(const char* s);
void die(const char* fmt, ...) __attribute__((noreturn));

int nocall(const char* f);

#define NOCALL(f) \
	void f() { nocall(#f); }
