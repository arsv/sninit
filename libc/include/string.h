#ifndef STRING_H
#define STRING_H

#include <sys/cdefs.h>
#include <stddef.h>

char *strcpy(char* restrict dest, const char* restrict src) nonnull((1,2));
int strcmp(const char *s1, const char *s2) pure nonnull((1,2));
unsigned long strlen(const char *s) pure nonnull((1));
char *strncpy(char* restrict dest, const char* restrict src, size_t n) nonnull((1,2));
char *strcat(char* restrict dest, const char* restrict src) nonnull((1,2));

void* memset(void* s, int c, size_t n) nonnull((1));
void* memcpy(void* restrict dest, const void* restrict src, size_t n) nonnull((1,2));
void *memccpy(void* restrict dest, const void* restrict src, int c, size_t n) nonnull((1,2));
int memcmp(const void* s1, const void* s2, size_t n) pure nonnull((1,2));

char *strerror(int errnum) __attribute__((const));

char *strpbrk(const char *s, const char *_accept) nonnull((1,2));
char *strsep(char ** restrict stringp, const char * restrict delim) nonnull((1,2));
char *strchr(const char *s, int c) pure nonnull((1));

#endif
