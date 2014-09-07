#ifndef CDEFS_H
#define CDEFS_H

#define pure __attribute__ ((__pure__))
#define nonnull(params) __attribute__ ((__nonnull__ params))
#define restrict __restrict__

#if __GNUC__ < 3
#define likely(foo) (foo)
#define unlikely(foo) (foo)
#else
#define likely(foo) __builtin_expect((foo),1)
#define unlikely(foo) __builtin_expect((foo),0)
#endif

/* idea for these macros taken from Linux kernel */

#endif
