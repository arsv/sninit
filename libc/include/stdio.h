#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

int snprintf(char *str, size_t size, const char *format, ...) __attribute__((__format__(__printf__,3,4)));
int vsnprintf(char *str, size_t size, const char *format, va_list ap) __attribute__((__format__(__printf__,3,0)));

#endif
