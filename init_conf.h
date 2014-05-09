typedef int offset;

struct stringnode {
	offset next;
	char str[0];
};

struct stringlist {
	int count;
	offset head;
	offset last;
};

struct scratch {
	int newend;
	int oldend;
	struct stringlist env;
};

/* mmaped blocks -- see init_mmem.c */
struct memblock {
	void* addr;		/* base address */
	int len;		/* allocated length */
	int ptr;		/* current pointer (=first unused byte offset) */
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
#define SCR ((struct scratch*)(newblock.addr + sizeof(struct config)))
/*

Now NCF/SCR should have looked like this:

	#define NCF ((struct dynconfig*) newblock.addr)

	struct dynconfig {
		struct config;
		// struct scratch contents
		int newend;
		struct stringlist env;
	}

but this is apparently considered waaay-non-standard and requires -fms-extensions
for gcc, and -fms-extensions -Wno-microsoft for clang.
So it's two pointers for now.

Note: SCR is inside of newblock now, no more scratchblock.

*/

#define blockptr(block, offset, type) ((type)((block)->addr + offset))
#define initrecat(block, offset) blockptr(block, offset, struct initrec*)

/* Some versions of musl (0.9.14 but not 0.9.10) #define NULL as 0L,
   apparently to cater to C++ folk. Surprisingly, C standard allows this.
   C compilers are less than happy with such a turn though, especially when
   rebasing pointers: (ptr1 + (ptr2 - NULL)). This only matters for conf
   code, the rest of init is ok with 0L. */
#ifdef NULL
#undef NULL
#endif
#define NULL ((void*)0)
