#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#include <sys/cdefs.h>

extern char **environ;

int atoi(const char *nptr);

char *getenv(const char *name) pure;
int putenv(const char *string);
int setenv(const char *name, const char *value, int overwrite);

#ifdef __GNUC__
#define alloca(x) __builtin_alloca(x)
#else
void *alloca(size_t size);
#endif

#endif
