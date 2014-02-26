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

extern int addinitrec(struct fileblock* fb, char* name, char* rlvl, char* flags, char* cmd, int exe);
extern int mextendblock(struct memblock* m, int size, int blocksize);
extern int scratchstring(char listcode, const char* string);

extern int mmapfile(struct fileblock* fb, int maxlen);
extern int munmapfile(struct fileblock* fb);
extern int nextline(struct fileblock* f);

static inline int skipdirent(struct dirent64* de);
int readsrvfile(char* fullname, char* basename, unsigned short defrlvl);
int parsesrvfile(struct fileblock* fb, char* basename);
int mkreldirname(char* buf, int len, const char* base, const char* dir);

/* bb = base block, for the file that included this directory */
int readinitdir(struct fileblock* bb, const char* dir, int defrlvl, int strict)
{
	int dirfd;
	struct dirent64* de;
	int nr, ni;		/* getdents ret and index */
	int ret = -1;

	char debuf[DENTBUFSIZE];
	char fname[FULLNAMEMAX];
	const int delen = sizeof(debuf);
	const int fnlen = sizeof(fname);
	int bnoff;		/* basename offset in fname */
	int bnlen;
	struct fileblock fb = { .name = fname, .rlvl = defrlvl };

	/*        |            |<---- bnlen ---->| */
	/* fname: |/path/to/dir/filename         | */
	/*        |--- bnoff ---^                  */
	if((bnoff = mkreldirname(fname, fnlen, bb->name, dir)) < 0)
		retwarn(ret, "%s:%i: directory name too long");
	bnlen = fnlen - bnoff;

	if((dirfd = open(fname, O_RDONLY | O_DIRECTORY)) < 0)
		retwarn(ret, "%s:%i: can't open %s, skipping", bb->name, bb->line, fname);

	while((nr = getdents64(dirfd, (void*)debuf, delen)) > 0) {
		for(ni = 0; ni < nr; ni += de->d_reclen) {
			de = (struct dirent64*)(debuf + ni);

			if(skipdirent(de))
				continue;

			strncpy(fname + bnoff, de->d_name, bnlen);

			if(!(ret = mmapfile(&fb, 1024))) {
				ret = parsesrvfile(&fb, de->d_name);
				munmapfile(&fb);
			} if(ret && strict)
				goto out;
			else if(ret)
				warn("%s:%i: skipping %s", bb->name, bb->line, de->d_name);
		}
	}

	ret = 0;
out:	close(dirfd);
	return ret;
}

static inline int skipdirent(struct dirent64* de)
{
	char dt;
	int len = strlen(de->d_name);

	/* skip hidden files */
	if(de->d_name[0] == '.')
		return 1;

	/* skip temp files */
	if(len > 0 && de->d_name[len - 1] == '~')
		return 1;

	/* skip non-regular files early if the kernel was kind enough to warn us */
	if((dt = de->d_type) && dt != DT_LNK && dt != DT_REG)
		return 1;

	return 0;
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

int parsesrvfile(struct fileblock* fb, char* basename)
{
	int shebang = 0;
	char* rlvls = NULL;
	char* flags = NULL;
	char* cmd;

	if(!nextline(fb))
		retwarn(-1, "%s: empty file", fb->name);

	/* Check for, and skip #! line if present */
	if(!strncmp(fb->ls, "#!", 2)) {
		shebang = 1;
		if(!nextline(fb))
			retwarn(-1, "%s: empty script", fb->name);
	}

	/* Do we have #: line? If so, note runlevels and flags */
	if(!strncmp(fb->ls, "#:", 2)) {
		char* il = fb->ls + 2;		/* skip #: */
		rlvls = strsep(&il, ":");
		flags = strsep(&il, ":");
		if(!nextline(fb))
			retwarn(-1, "%s: no command found", fb->name);
	}

	if(shebang) {
		/* No need to parse anything anymore, it's a script. */
		/* Note: for an initdir entry, fb->name is in readinitdir() stack
		   and thus writable */
		cmd = (char*)fb->name;
	} else {
		/* Get to first non-comment line, and that's it, the rest
		   will be done in addinitrec. */
		while(!*(fb->ls) || *(fb->ls) == '#')
			if(!nextline(fb))
				retwarn(-1, "%s: no command found", fb->name);

		cmd = fb->ls;
	}

	return addinitrec(fb, basename, rlvls, flags, cmd, shebang);
}
