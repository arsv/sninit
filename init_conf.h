typedef int offset;

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

/* The three mmaped blocks. See init_conf_map.c.
   These should have been anonymous structures, but they are declared
   extern in several files. */

struct cblock {
	void* addr;
	int len;
}; /* cfgblock */

struct nblock {
	void* addr;
	int len;
	int ptr;
}; /* newblock */

struct fblock {
	char* buf;
	int len;

	const char* name;
	int line;

	char* ls;
	char* le;
}; /* fileblock */

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

#define NCF newblockptr(0, struct config*)
#define SCR newblockptr(sizeof(struct config), struct scratch*)
#define FBN fileblock.name
#define FBL fileblock.line

/* Offsets of scratch.{inittab,env} within newblock, for linknode */
#define TABLIST (sizeof(struct config) + offsetof(struct scratch, inittab))
#define ENVLIST (sizeof(struct config) + offsetof(struct scratch, env))

/* For addptrsarray */
#define NULL_FRONT (1<<0)
#define NULL_BACK  (1<<1)
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

extern struct nblock newblock;
extern struct fblock fileblock;

/* top-level functions handling configuration */
extern int readinittab(const char* file, int strict);
extern int readinitdir(const char* dir, int strict);

extern int mmapblock(int size);
extern void munmapblock(void);
extern offset extendblock(int size);

extern void exchangeblocks(void);
