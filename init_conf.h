typedef int offset;

/* mmaped blocks -- see init_mmem.c */
struct memblock {
	void* addr;		/* base address */
	int len;		/* allocated length */
	int ptr;		/* current pointer (=first unused byte offset) */
};

/* A linked list of ptrnode-s */
struct ptrlist {
	offset head;
	offset last;
	int count;
};
struct ptrnode {
	offset next;
	/* void payload[]; but alas we can't write this in C */
};
/* payload is actually either initrec (for scrath.inittab)
   or an env string (scrach.env) */

struct scratch {
	struct ptrlist inittab;
	struct ptrlist env;
};

/* mmaped file, to be parsed in linewise manner */
struct fileblock {
	const char* name;
	int line;

	char* buf;
	int len;

	char* ls;
	char* le;
};

/* newblock layout looks like this:

       struct config;		<- NCF
       struct scratch;		<- SCR
       ptrnode
       initrec
       ptrnode
       initrec
       ptrnode
       envline
       ptrnode
       initrec
       ...
       inittab[]
       envp[]

   Trailing arrays are only added in finishinittab */

#define newblockptr(offset, type) ((type)(newblock.addr + offset))

#define CFG ((struct config*) cfgblock.addr)
#define NCF ((struct config*) newblock.addr)
#define SCR newblockptr(sizeof(struct config), struct scratch*)

/* Offsets of scratch.{inittab,env} within newblock, for linknode */
#define TABLIST (sizeof(struct config) + offsetof(struct scratch, inittab))
#define ENVLIST (sizeof(struct config) + offsetof(struct scratch, env))

/* For addptrsarray */
#define NULL_FRONT (1<<0)
#define NULL_BACK (1<<1)
#define NULL_BOTH (NULL_FRONT | NULL_BACK)

/* Some versions of musl (0.9.14 but not 0.9.10) #define NULL as 0L,
   apparently to cater to C++ folk. Surprisingly, C standard allows this.
   C compilers are less than happy with such a turn though, especially when
   rebasing pointers: (ptr1 + (ptr2 - NULL)). This only matters for conf
   code, the rest of init is ok with 0L. */
#ifdef NULL
#undef NULL
#endif
#define NULL ((void*)0)
