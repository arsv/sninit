#include <stdio.h>

/* test if success (zero) */
#define T(a) {\
	int r = a;\
	if(r)\
		printf("%s:%i: FAIL(%i) %s\n", __FILE__, __LINE__, r, #a);\
	else\
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, #a);\
}

/* assert (test if non-zero) */
#define A(a) \
	if(a)\
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, #a);\
	else\
		printf("%s:%i: FAIL %s\n", __FILE__, __LINE__, #a);

/* same, with custom message */
#define Ac(a, fmt, ...) \
	if(a)\
		printf("%s:%i: OK " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);\
	else\
		printf("%s:%i: FAIL " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);

/* compare strings */
#define S(a,b) {\
	const char* r = a;\
	if(b == NULL && r == NULL)\
		printf("%s:%i: OK %s = NULL\n", __FILE__, __LINE__, #a);\
	else if(r == NULL) \
		printf("%s:%i: FAIL %s = NULL\n", __FILE__, __LINE__, #a);\
	else if(strcmp(r, b)) \
		printf("%s:%i: FAIL %s = \"%s\"\n", __FILE__, __LINE__, #a, r);\
	else\
		printf("%s:%i: OK %s = \"%s\"\n", __FILE__, __LINE__, #a, b);\
}

/* check pointer p against block b */
#define B(b, p) {\
	if(p && !((void*)p >= b.addr && (void*)p <= b.addr + b.len))\
		printf("%s:%i: FAIL %s=%p %s=[%p +%i]\n", __FILE__, __LINE__, #p, p, #b, b.addr, b.len);\
	else\
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, #p);\
}

/* check pointer p against block b */
#define Bc(b, p, fmt, ...) {\
	if(p && !((void*)p >= b.addr && (void*)p <= b.addr + b.len))\
		printf("%s:%i: FAIL " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);\
	else\
		printf("%s:%i: OK " fmt "\n", __FILE__, __LINE__, __VA_ARGS__);\
}
