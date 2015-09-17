#include <string.h>
#include "init.h"
#include "scope.h"

extern int currlevel;
extern int nextlevel;
extern struct config* cfg;

extern int levelmatch(struct initrec* p, int level);
local void dumprec(struct initrec* p);
local void joincmd(char* buf, int len, char** argv);
local void rlstr(char* str, int len, int mask);

/* These two functions provide telinit pidof and telinit ? output
   respectively. Both are called from within parsecmd, with warnfd
   being the open telinit connection.

   This is the only case when warn() is used for non-error output. */

void dumpidof(struct initrec* p)
{
	if(p->pid > 0)
		warn("%i", p->pid);
}

void dumpstate(void)
{
	struct initrec *p, **pp;
	bss char currstr[16];
	bss char nextstr[16];

	rlstr(currstr, 16, currlevel);
	rlstr(nextstr, 16, nextlevel);
	if(currlevel == nextlevel)
		warn("Runlevel %s", currstr);
	else
		warn("Switching %s to %s", currstr, nextstr);

	for(pp = cfg->inittab; (p = *pp); pp++) {
		if(p->flags & C_ONCE)
			if(currlevel == nextlevel)
				continue;
		if(!levelmatch(p, nextlevel) && p->pid <= 0)
			continue;

		dumprec(p);
	}
}

/* The code below will not give a perfect output, far from it.
   Before improving it however, think whether init is the right
   place to practice elaborate text output formatting.

   Showing detailed process state (flags) here is not optional,
   the user should have some idea as to why a particular process
   is not running.

   Pretty much all output formatting here should have been in telinit.
   However, init would still need to format the data for telinit
   to parse, and with minimal effort said formatting happens to be
   human-readable. So why bother. */

void dumprec(struct initrec* p)
{
	bss char cmdbuf[MAXREPORTCMD];
	joincmd(cmdbuf, sizeof(cmdbuf), p->argv);
	char ext[2] = { '\0', '\0' };

	if(p->flags & P_MANUAL)
		*ext = '+';
	else if(p->flags & P_FAILED)
		*ext = 'x';
	else if(p->pid < 0)
		*ext = '-';
	else if(p->flags & (P_SIGTERM | P_SIGKILL))
		*ext = '!';
	else if(p->flags & P_SIGSTOP)
		*ext = '*';

	if(p->pid > 0)
		warn("%s\t%i%s\t%s", p->name, p->pid, ext, cmdbuf);
	else
		warn("%s\t%s\t%s",  p->name, ext, cmdbuf);
}

/* Convert runlevel bitmask into a readable string:
       (1<<2) | (1<<a) | (1<<c) -> "1ac"
   Telinit will show this as "current runlevel" */

void rlstr(char* str, int len, int mask)
{
	char* p = str;
	char* end = str + len - 1;
	static char bits[16] = "0123456789abcdef";
	int i;

	for(i = 0; i < 16 && p < end; i++)
		if(mask & (1 << i))
			*(p++) = bits[i];

	*p = '\0';
}

/* Join argv into a single string, to be reported by "telinit ?"
   The string is cut, with ... added at the end, if it does not
   fit in the output buffer. */

void joincmd(char* buf, int len, char** argv)
{
	char** arg;
	int arglen;
	int cpylen;
	char* ptr = buf;

	if(len < 4)	/* "...\0"; should never happen */
		goto out;

	len--; /* terminating \0 */
	for(arg = argv; *arg && len > 0; arg++) {
		arglen = strlen(*arg);
		cpylen = (arglen <= len ? arglen : len);

		strncpy(ptr, *arg, cpylen);
		ptr += cpylen;
		len -= cpylen;

		if(cpylen < arglen) {
			break;
		} else if(len > 0) {
			*(ptr++) = ' ';
			len--;
		}
	} if(*arg) {
		ptr += (len > 3 ? 3 : len);
		strncpy(ptr - 3, "...", 3);
	} else if(ptr > buf && *(ptr-1) == ' ')
		ptr--;

out:	*ptr = '\0';
}
