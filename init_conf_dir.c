/* Parsing initdir files */

#define _GNU_SOURCE
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "config.h"
#include "init.h"
#include "init_conf.h"
#include "sys.h"

extern int addinitrec(struct fileblock* fb, char* code, char* name, char* cmd, int exe);

extern int mmapfile(struct fileblock* fb, int maxlen);
extern int munmapfile(struct fileblock* fb);
extern int nextline(struct fileblock* f);

static inline int skipdirent(struct dirent64* de);
static int parsesrvfile(struct fileblock* fb, char* basename);

/* bb = base block, for the file that included this directory */
int readinitdir(const char* dir, int strict)
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
	struct fileblock fb = { .name = fname };

	/*        |            |<---- bnlen ---->| */
	/* fname: |/path/to/dir/filename         | */
	/*        |--- bnoff ---^                  */
	strncpy(fname, dir, fnlen - 1);
	bnoff = strlen(fname);
	fname[bnoff++] = '/';
	bnlen = fnlen - bnoff;

	if((dirfd = open(dir, O_RDONLY | O_DIRECTORY)) < 0) {
		if(errno == ENOENT)
			return 0;
		else
			retwarn(ret, "Can't open %s", dir);
	}

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
				warn("%s: skipping %s", dir, de->d_name);
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

	/* skip uppercase files (this is bad but keeps README out for now) */
	if(len > 0 && de->d_name[0] >= 'A' && de->d_name[0] <= 'Z')
		return 1;

	/* skip non-regular files early if the kernel was kind enough to warn us */
	if((dt = de->d_type) && dt != DT_LNK && dt != DT_REG)
		return 1;

	return 0;
}

/* Make a full directory name for $dir when included from a file named $base.
	base="/etc/inittab" dir="services" -> "/etc/services"
   Returns basename offset, see the call above */

static int parsesrvfile(struct fileblock* fb, char* basename)
{
	int shebang = 0;
	char* code;
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
		code = fb->ls + 2;	/* skip #: */
		if(!nextline(fb))
			retwarn(-1, "%s: no command found", fb->name);
	} else {
		code = "";
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

	return addinitrec(fb, code, basename, cmd, shebang);
}
