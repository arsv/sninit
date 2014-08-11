/* For cases when it's explicitly offset.
   Most of the time however, it's just NULL-based void* */
typedef int offset;

/* mmaped blocks -- see init_mmem.c */
struct memblock {
	void* addr;		/* base address */
	int len;		/* allocated length */
	int ptr;		/* current pointer (=first unused byte offset) */
};

/* Same structure both for inittab and envp.
   There are more efficient ways to do it, but since we're talking about
   maybe a page or so, why bother. */
/* Pointer are in fact offsets. */
struct ptrnode {
	offset next;	/* relative to scratchblock */
	offset ptr;	/* relative to newblock */
};

struct ptrlist {
	offset head;	/* both in scratchblock */
	offset last;
	int count;
};

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

	/* Default runlevel mask. Looks completely out of place here,
	   but actually fits quite well, given struct fileblock is more
	   like parsing context than just an arbitrary mmaped file. */
	unsigned short rlvl;
};

#define CFG ((struct config*) cfgblock.addr)
#define NCF ((struct config*) newblock.addr)
#define SCR ((struct scratch*) scratchblock.addr)

#define blockptr(block, offset, type) ((type)((block)->addr + offset))

/* For scratchptr() */
#define TABLIST offsetof(struct scratch, inittab)
#define ENVLIST offsetof(struct scratch, env)

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
