#include <sys/types.h>
#include <string.h>

int memcmp(const void *dst, const void *src, size_t count) {
  register int r;
  register const unsigned char *d=dst;
  register const unsigned char *s=src;
  ++count;
  while (likely(--count)) {
    if (unlikely(r=(*d - *s)))
      return r;
    ++d;
    ++s;
  }
  return 0;
}
