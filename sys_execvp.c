#define _GNU_SOURCE
/* ^ needed for char** environ to be defined in glibc, whoa */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* execvp from dietlibc does some undesired things like trying
   to invoke shell in some cases. This one doesn't.
 
   Only called in runcap atm; init can not rely on $PATH
   and telinit does not exec anything. */

int execvp(const char *file, char *const argv[])
{
	/* short-circuit to execve when something resembling path is supplied */
	if(strchr(file,'/'))
		return execve(file, argv, environ);

	const char* p = getenv("PATH");

	/* it's a command and there's no $PATH defined */
	if(!p) goto out;

	char buf[PATH_MAX];
	const char* e;
	int fl = strlen(file);
	int pl;

	while(*p) {
		for(e = p; *e && *e != ':'; e++);

		/* will full path fit in buf? */
		if((pl = e - p) + 2 + fl > PATH_MAX)
			continue;

		memcpy(buf, p, pl);
		buf[pl] = '/';
		memcpy(buf + pl + 1, file, fl);
		buf[pl + 1 + fl] = '\0';

		execve(buf, argv, environ);

		/* we're still here, so execve failed */
		if((errno != EACCES) && (errno != ENOENT) && (errno != ENOTDIR))
			return -errno;

		p = *e ? e + 1 : e;
	}
out:	
	return -(errno = ENOENT);
}
