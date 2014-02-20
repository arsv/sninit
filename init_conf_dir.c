/* Parsing initdir files */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "config.h"
#include "init.h"
#include "init_conf.h"
#include "sys.h"

extern int addinitrec(struct fileblock* fb, char* name, int diri, char* rlvl, char* flags, char* cmd, int exe);
extern int mextendblock(struct memblock* m, int size, int blocksize);
extern int scratchstring(char listcode, const char* string);

extern int mmapfile(struct fileblock* fb, int maxlen);
extern int munmapfile(struct fileblock* fb);
extern int nextline(struct fileblock* f);

int allocdirbuf(int desize, int fnsize, int extralen);
int readsrvfile(char* fullname, char* basename, int diri);
int parsesrvfile(struct fileblock* fb, char* basename, int diri);
int mkreldirname(char* buf, int len, const char* base, const char* dir);

/* bb = base block, for the file that included this directory */
int readinitdir(struct fileblock* bb, const char* dir, int strict)
{
	int dirfd;
	struct dirent64* de;
	char dt;		/* dirent type; XXX: 0 to keep valgrind happy */
	int nr, ni;		/* getdents ret and index */
	int diri;		/* index in SCR->dir list */
	int ret = -1;

	char debuf[DENTBUFSIZE];
	char fname[FULLNAMEMAX];
	const int delen = sizeof(debuf);
	const int fnlen = sizeof(fname);
	int bnoff;		/* basename offset in fname */
	int bnlen;

	/*        |            |<---- bnlen ---->| */
	/* fname: |/path/to/dir/filename         | */
	/*        |--- bnoff ---^                  */
	if((bnoff = mkreldirname(fname, fnlen, bb->name, dir)) < 0)
		retwarn(ret, "%s:%i: directory name too long");
	bnlen = fnlen - bnoff;

	if((dirfd = open(fname, O_RDONLY | O_DIRECTORY)) < 0)
		retwarn(ret, "%s:%i: can't open %s, skipping", bb->name, bb->line, fname);

	if((diri = scratchstring('D', dir)) < 0)
		gotowarn(out, "%s:%i: can't add directory to dirlist", bb->name, bb->line, dir);

	/* Warning: both fn and de->d_name iside the loop reside in scratch.
	   Any reallocations of scratch block invalidate these pointers.
	   readsrvfile/parsesrvfile must decouple both
	   if addinitrec() does anything with scratchblock (it doesn't currently) */
	while((nr = getdents64(dirfd, (void*)debuf, delen)) > 0) {
		for(ni = 0; ni < nr; ni += de->d_reclen) {
			de = (struct dirent64*)(debuf + ni);

			/* skip hidden files */
			if(de->d_name[0] == '.')
				continue;

			/* skip non-regular files early if the kernel was kind enough to warn us */
			if((dt = de->d_type) && dt != DT_LNK && dt != DT_REG)
				continue;

			strncpy(fname + bnoff, de->d_name, bnlen);
			ret = readsrvfile(fname, de->d_name, diri);

			if(ret)
				warn("%s:%i: skipping %s", bb->name, bb->line, de->d_name);
			if(ret && strict)
				goto out;
		}
	}

	ret = 0;
out:	close(dirfd);
	return ret;
}

/* Make a full directory name for $dir when included from a file named $base.
	base="/etc/inittab" dir="services" -> "/etc/services"
   Returns basename offset, see the call above */

/* $base is always the same, at least as long as there are no recursive includes.
   However, to keep INITTAB a single constant (vs. having INITTAB="/etc/inittab"
   and INITBASE="/etc/"), and also since it changes almost nothing, this function
   does not make any assumption regarding $base value. */

int mkreldirname(char* buf, int len, const char* base, const char* dir)
{
	int dirlen = strlen(dir);
	char* p = buf;
	const char* q;

	/* Now this all can be done with stpncpy and strlen, but the result
	   is just as ugly and about as long as well. */
	if(*dir != '/') {		/* relative path, use "$base/$dir/" */
		/* copy all $base */
		for(q = base; p < buf + len && *q; p++, q++)
			*p = *q;
		/* backtrack to the rightmost /, separating dirname */
		do p--; while(p > buf && *p != '/');
		/* if there was a slash, keep it */
		if(*p == '/')
			p++;
		/* now it's clear now much space is needed, so make sure it's there */
		if(p - buf + dirlen + 2 > len)
			return -1;
	} else if(dirlen > len - 2)	/* absolute path, "$dir/" */
		return -1;

	/* append $dir */
	for(q = dir; p < buf + len && *q; p++, q++)
		*p = *q;
	/* append trailing /, but only if it's not there already */
	if(p > buf && *(p - 1) != '/')
		*p++ = '/';
	*p = '\0';

	return p - buf;
}

int readsrvfile(char* fullname, char* basename, int diri)
{
	struct fileblock fb = { .name = fullname };

	if(mmapfile(&fb, 1024))
		return -1;

	int ret = parsesrvfile(&fb, basename, diri);
	munmapfile(&fb);

	return ret;
}

int parsesrvfile(struct fileblock* fb, char* basename, int diri)
{
	int shebang;
	char* rlvls;
	char* flags;
	char* cmd;

	if(!nextline(fb))
		retwarn(-1, "%s: empty file", fb->name);

	if(!strncmp(fb->ls, "#!", 2)) {
		shebang = 1;
		if(!nextline(fb))
			retwarn(-1, "%s: empty script", fb->name);
	} else {
		shebang = 0;
	}

	if(strncmp(fb->ls, "#:", 2))
		retwarn(0, "%s: doesn't look like a service file, skipping", fb->name);

	char* il = fb->ls + 2;		/* initline, the part following #: */
	rlvls = strsep(&il, ":");
	flags = strsep(&il, ":");

	if(shebang) {
		/* No need to parse anything anymore, it's a script. */
		/* Note: for an initdir entry, fb->name is in readinitdir() stack
		   and thus writable */
		cmd = (char*)fb->name;
	} else {
		char* ilend = fb->le;
		char* fbend = fb->buf + fb->len;

		cmd = il;	/* il != NULL after strsep()s above means there was command part in il */

		/* in case of a mixed command (both in and after initline), revert line-end terinator
		   so that addinitrec() later would see a continous multi-line string */
		if(ilend < fbend && cmd)		/*	#:123:null:!	*/
			*ilend = '\n';			/*	echo $PATH	*/

		else if(ilend < fbend)			/*	#:123:null 	and something on the next line */
			cmd = ilend + 1;

		/* else do nothing, cmd is either the command, or NULL if there's no command at all */

		/* Now, this all may leave some extra spaces/newlines in cmd.
		   That's ok and will be handled later in or near prepargv(). */
	}

	return addinitrec(fb, basename, diri, rlvls, flags, cmd, shebang);
}
