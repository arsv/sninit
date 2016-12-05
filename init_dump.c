#include <string.h>
#include "config.h"
#include "init.h"
#include "scope.h"

static char cmdbuf[MAXREPORTCMD];
static char currstr[16];
static char nextstr[16];

/* Run-once entries should only be shown when the init is switching
   runlevels, as it may help determining what's going on and why we're not
   on the target runlevel yet.

   Otherwise, we've got to check both nextlevel *and* currlevel to
   show entries that are dying. */

static int shouldbeshown(struct initrec* p)
{
	int switching = (currlevel != nextlevel);

	if(p->pid > 0)	/* anything running is worth showing */
		return 1;
	if((p->flags & C_ONCE) && !switching)
		return 0;

	return levelmatch(p, nextlevel) ||
	(switching ? levelmatch(p, currlevel) : 0);
}

/* Expected string length of a positive integer (entry pid) */

static int pintlen(int n)
{
	int l = 0;
	while(n > 0) { l++; n /= 10; }
	return l;
}

/* For paused/disabled/failed/signalled entries, we show a sign next to
   or instead of its pid. This is the only way for the user to know why
   a certain process is not running. */

static char rectag(struct initrec* p)
{
	if(p->flags & P_MANUAL)
		return '-';
	else if(p->flags & P_FAILED)
		return 'x';
	else if(p->pid < 0)
		return '-';
	else if(p->flags & (P_SIGTERM | P_SIGKILL))
		return '!';
	else if(p->flags & P_SIGSTOP)
		return '*';
	else
		return ' ';
}

/* Convert runlevel bitmask into a readable string:
       (1<<2) | (1<<a) | (1<<c) -> "1ac"
   Telinit will show this as "current runlevel" */

static void rlstr(char* str, int len, int mask)
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

/* warn() can't handle argv[] directly, and it can be too long to show anyway.

   [ "/sbin/vsftpd", "/etc/vsftpd.conf", "-obackground=NO" ] ->
           "/sbin/vsftpd /etc/vsftpd.conf -oback..."

   The string is written to a temporary buffer, which is then passed
   to warn as %s.

   Command width is a constant set in config.h. There is no point in trying
   to show the whole command, or even as much of the command as possible.
   The first few args should be informative enough. */

static void joincmd(char* buf, int len, char** argv)
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

	/* sanitize the string */
	for(ptr = buf; *ptr; ptr++)
		if(*ptr < 0x20)
			*ptr = ' ';
}

static void dumprec(struct initrec* p, int namewidth, int pidwidth)
{
	char tag = rectag(p);

	if(p->flags & C_SHELL)
		/* C_SHELL implies argv = [ /bin/sh, -c, command, NULL ] */
		joincmd(cmdbuf+1, sizeof(cmdbuf)-1, p->argv+2), *cmdbuf = '!';
	else
		joincmd(cmdbuf,   sizeof(cmdbuf),   p->argv);

	if(p->pid > 0)
		warn("%-*s    %c%-*i    %s",
			namewidth, p->name, tag, pidwidth, p->pid, cmdbuf);
	else
		warn("%-*s     %-*c    %s",
			namewidth, p->name, pidwidth, tag, cmdbuf);
}

/* These two functions provide telinit pidof and telinit ? output
   respectively. Both are called from within parsecmd, with warnfd
   being the open telinit connection.

   This is the only case when warn() is used for non-error output.
   The leading # sign is used to pass that to telinit, making
   it choose stdout instead of stderr and exit with code 0.

   How it should look like:

       # telinit pidof sshd
       1234

       # telinit list
       Runlevel 3a
       ftpd        546     /usr/bin/vsftpd /etc/vsftpd.conf ...
       dropbear    1234    /usr/sbin/dropbear -F -R
       ntpd        -       /usr/sbin/ntpd -g -n

   The code below takes some effort to align columns properly,
   otherwise the output look really bad. Init is not the place to
   do fancy text formatting, but other options are worse in various
   ways. */

void dumpidof(struct initrec* p)
{
	if(p->pid > 0)
		warn("#%i", p->pid);
}

void dumpstate(void)
{
	struct initrec *p, **pp;

	rlstr(currstr, 16, currlevel);
	rlstr(nextstr, 16, nextlevel);
	if(currlevel == nextlevel)
		warn("#Runlevel %s", currstr);
	else
		warn("#Switching %s to %s", currstr, nextstr);

	/* First pass, get column widths */

	int len;
	int maxnamelen = 0;
	int maxpidlen = 0;

	for(pp = cfg->inittab; (p = *pp); pp++) {
		if(!shouldbeshown(p))
			continue;
		if((len = strlen(p->name)) > maxnamelen)
			maxnamelen = len;
		if((len = pintlen(p->pid)) > maxpidlen)
			maxpidlen = len;
	}

	/* Second pass, do the output */

	for(pp = cfg->inittab; (p = *pp); pp++)
		if(shouldbeshown(p))
			dumprec(p, maxnamelen, maxpidlen);
}
