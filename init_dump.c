#include <string.h>
#include "init.h"
#include "scope.h"

extern int currlevel;
extern int nextlevel;
extern struct config* cfg;

local char* joincmd(char* buf, int len, char** argv);
local void rlstr(char* str, int len, int mask);

/* These two functions provide telinit pidof and telinit ? output
   respectively. Both are called from within parsecmd, with warnfd
   being an open telinit connection.

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
	bss char cmdbuf[MAXREPORTCMD];
	char* reportcmd;

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
		reportcmd = p->name[0] ? p->name : joincmd(cmdbuf, sizeof(cmdbuf), p->argv);
		if(p->pid > 0)
			warn("%i\t%s", p->pid, reportcmd);
		else
			warn("%s\t%s", "-", reportcmd);
	}
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

	for(i = 0; i < 16; i++)
		if(mask & (1 << i)) {
			*p = bits[i];
			if(p++ >= end - 1) break;
		}

	*p = '\0';
}

/* Join argv into a single string, to be reported by "telinit ?"
   The string is cut, with ... added at the end, if it does not
   fit in the output buffer. */

char* joincmd(char* buf, int len, char** argv)
{
	char** arg;
	int arglen;
	int cpylen;
	char* ret = buf;

	if(len < 4)	/* "...\0"; should never happen */
		goto out;

	len--; /* terminating \0 */
	for(arg = argv; *arg && len > 0; arg++) {
		arglen = strlen(*arg);
		cpylen = (arglen <= len ? arglen : len);

		strncpy(buf, *arg, cpylen);
		buf += cpylen;
		len -= cpylen;

		if(cpylen < arglen) {
			break;
		} else if(len > 0) {
			*(buf++) = ' ';
			len--;
		}
	} if(*arg) {
		buf += (len > 3 ? 3 : len);
		strncpy(buf - 3, "...", 3);
	} else if(buf > ret && *(buf-1) == ' ')
		buf--;

out:	*buf = '\0';
	return ret;
}
