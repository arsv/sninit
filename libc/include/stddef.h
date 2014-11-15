#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void*)0)

#include <bits/types.h>

#define offsetof(type,member) ((size_t) &((type*)0)->member)

#endif
