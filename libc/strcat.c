#include <string.h>

char* strcat(register char* s,register const char* t)
{
  char *dest=s;
  s+=strlen(s);
  for (;;) {
    if (!(*s = *t)) break; ++s; ++t;
  }
  return dest;
}

