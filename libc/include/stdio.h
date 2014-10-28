#ifndef STDIO_H
#define STDIO_H

#include <sys/types.h>
#include <stdarg.h>

int printf(const char *format, ...) __attribute__((__format__(__printf__,1,2)));
int snprintf(char *str, size_t size, const char *format, ...) __attribute__((__format__(__printf__,3,4)));
int vsnprintf(char *str, size_t size, const char *format, va_list ap) __attribute__((__format__(__printf__,3,0)));
int vprintf(const char *format, va_list ap) __attribute__((__format__(__printf__,1,0)));

#endif
