#include <string.h>
#include <stddef.h>
#include "init.h"
#include "init_conf.h"

/* Once the static part of initrec structure has been placed within
   newblock by addinitrec, we've got append a proper argv[] array.

   By this point, newblock.ptr is right where the start of argv[]
   should be, and cmd points into a fileblock.
   For the command itself, there are three options:

	(1) exe=0 argv="/sbin/httpd -f /etc/httpd.conf"
	(2) exe=0 argv="echo foo > /sys/blah/something"
	(3) exe=1 argv="/etc/rc/script"

   Executable initdir entry (3) can be used as is, simple command (1)
   should be split into argv[] here, and not-so-simple commands like (2)
   are passed to /bin/sh -c.

   (2) and (3) need no string manipulation and share the same logic,
   but (1) requires some effort to parse the string.

   For (2), bb init attempts to inject "exec" befor the command.
   This is not done here. See doc/misc.txt for explaination. */

/* The entry pointer is fragile, add*array calls invalidate it.
   However, this whole function depends on newblock.ptr not moving
   after the static of the entry has been added, so it's just as fragile
   itself. The pointer is only needed to make dumpstate() output pretty,
   so why bother. See addinitrec(). */

static int addstring(const char* s)
{
	int l = strlen(s);
	int o = extendblock(l + 1);

	if(o < 0) return -1;

	char* p = newblockptr(o, char*);
	memcpy(p, s, l);
	p[l] = '\0';

	return o;
}

/* The easier case, laying out a pre-made [ "cmd", "arg1", "arg2", ... ]
   array for (2) or (3). The array is not NULL-terminated, its size
   is always known statically. */

static int addstaticargv(const char** args, int n)
{
	int i;
	offset argvo;	/* argv[] location in newblock */
	offset argio;	/* argv[i] string location in newblock */

	/* pointers array */
	if((argvo = extendblock((n+1)*sizeof(char*))) < 0)
		return -1;

	/* strings themselves */
	for(i = 0; i < n; i++) {
		if((argio = addstring(args[i])) < 0)
			return -1;
		newblockptr(argvo, char**)[i] = NULL + argio;
	}
	newblockptr(argvo, char**)[i] = NULL;

	return 0;
}

/* The harder case, parsing and laying out "cmd arg1 arg2 ..." string.

   The first pass here allocates the pointers array, argv[], filling
   it with valid pointers to the source strings. Then the second pass
   over newly-built argv[] copies the strings themselves to newblock
   and replaces the pointers with offsets within newblock. */

static int addparsedargv(char* str)
{
	int i;
	int argc = 0;

	char* argi;	/* argv[i] string location in the source file */
	offset argvi;	/* argv[i] pointer location in newblock */
	offset argio;	/* copied argv[i] string location in newblock */
	offset argvo = newblock.ptr;	/* argv[] location in newblock */

	do {
		argi = strssep(&str);
		if((argvi = extendblock(sizeof(char*))) < 0)
			return -1;
		*(newblockptr(argvi, char**)) = argi;
		argc++;

	} while(argi);

	for(i = 0; i < argc - 1; i++) {
		argi = newblockptr(argvo, char**)[i];
		if((argio = addstring(argi)) < 0)
			return -1;
		newblockptr(argvo, char**)[i] = NULL + argio;
	};
	newblockptr(argvo, char**)[i] = NULL;

	return 0;
}

/* Simple commands are execve()d directly, but if there is something
   the stupid parser in addstringarray won't handle well, we've got to run
   /bin/sh -c "cmd". The same logic is used in bb init and sysvinit.

   Not the best idea actually, but the absolute majority of inittab commands
   only need splitting on \s+ so doing some elaborate parsing instead
   results in a lot of dead code.

   Contents from initdir files ends up here as well, so cmd
   may happen to contain newlines. */

static int shellneeded(const char* cmd)
{
	char* p = strpbrk(cmd, "/ \t\n");
	if(!*p || *p != '/')
		return 1;

	return strpbrk(cmd, "'\"$;|><&{}\n") ? 1 : 0;
}

/* gcc has built-in prototype for this, with an int argument */

static int isspace(int c)
{
	switch(c) {
		case ' ':
		case '\t':
		case '\n': return 1;
		default: return 0;
	}
}

/* Like strsep(), but using /\s+/ for delimiter.
   Used here to split command line into argv[] elements, and
   init parseinittab to separate inittab fields.

   The function trims trailing whitespace only.
   Leading whitespace is significant in inittab, where strssep
   is expected to return "" for unnamed entries. */

char* strssep(char** str)
{
	char* ret = *str;
	char* ptr;

	if(!ret) return ret;

	for(ptr = ret; *ptr; ptr++)
		if(isspace(*ptr))
			break;
	while(isspace(*ptr))
		*(ptr++) = '\0';
	if(!*ptr)
		ptr = NULL;

	*str = ptr;
	return ret;
}

int addrecargv(struct initrec* entry, char* cmd, int exe)
{
	while(*cmd && isspace(*cmd)) cmd++;

	if(exe) {
		const char* argv[] = { cmd };
		return addstaticargv(argv, 1);
	} else if(shellneeded(cmd)) {
		const char* argv[] = { "/bin/sh", "-c", cmd };
		entry->flags |= C_SHELL;
		return addstaticargv(argv, 3);
	} else {
		return addparsedargv(cmd);
	}
}
