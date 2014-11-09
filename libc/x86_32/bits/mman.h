#ifndef BITS_MMAN_H
#define BITS_MMAN_H

#define MREMAP_MAYMOVE	1UL
#define MREMAP_FIXED	2UL

#define PROT_READ	0x1		/* page can be read */
#define PROT_WRITE	0x2		/* page can be written */
#define PROT_EXEC	0x4		/* page can be executed */
#define PROT_NONE	0x0		/* page can not be accessed */

#define MAP_SHARED	0x01		/* Share changes */
#define MAP_PRIVATE	0x02		/* Changes are private */
#define MAP_ANONYMOUS	0x20		/* don't use a file */

#define MAP_FAILED      ((void *) -1)

#endif
