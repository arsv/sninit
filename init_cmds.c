#include <string.h>
#include <signal.h>
#include <sys/reboot.h>
#include "init.h"

extern int state;
extern int currlevel;
extern int nextlevel;
extern int rbcode;

extern struct config* cfg;

extern struct initrec* findentry(const char* name);
extern void stop(struct initrec* p);
extern int configure(int strict);

global void parsecmd(char* cmd);

static void setrunlevel(const char* cmd);
static void dumpstate(void);

static void dorestart(struct initrec* p);
static void doenable(struct initrec* p, int v);
static void dopause(struct initrec* p, int v);
static void dohup(struct initrec* p, int v);

/* cmd here is what telinit sent to initctl.
   The actual command is always cmd[0], while cmd[1:] is (optional) argument.
   Examples:
	"q"		reconfigure
	"9"		switch to runlevel 9
	"dhttpd"	disable (stop) service named "httpd"

   warn() sends messages back to telinit */

void parsecmd(char* cmd)
{
	char* arg = cmd + 1;
	struct initrec* p = NULL;

	/* Runlevel switching commands handle argument differently */
	if((*cmd >= '0' && *cmd <= '9') || *cmd == '+' || *cmd == '-')
		return setrunlevel(cmd);

	switch(*cmd) {
		case 'r':		/* restart */
		case 'e': case 'd':	/* start, stop */
		case 'p': case 'w':	/* pause, resume */
		case 'h': case 'u':	/* hup, uphup */
			if(!(p = findentry(arg)))
				retwarn_("can't find %s in inittab", arg);
			break;
		default:
			if(*arg)
				retwarn_("no argument allowed for %c", *cmd);
	}

	switch(*cmd) {
		/* sleep levels */
		case 'Y': nextlevel = (nextlevel & SUBMASK) | (1 << 7); break;
		case 'S': nextlevel = (nextlevel & SUBMASK) | (1 << 8); break;
		case 'Z': nextlevel = (nextlevel & SUBMASK) | (1 << 9); break;
		/* halt */
		case 'H': nextlevel = 1; rbcode = RB_HALT_SYSTEM; break;
		case 'P': nextlevel = 1; rbcode = RB_POWER_OFF;   break;
		case 'R': nextlevel = 1; rbcode = RB_AUTOBOOT;    break;
		/* process ops */
		case 'r': dorestart(p); break;
		case 'p': dopause(p, 1); break;
		case 'w': dopause(p, 0); break;
		case 'd': doenable(p, 0); break;
		case 'e': doenable(p, 1); break;
		case 'h': dohup(p, 0); break;
		case 'u': dohup(p, 1); break;
		/* state query */
		case '?': dumpstate(); break;
		/* reconfigure */
		case 'q':
			if(configure(1))
				warn("!configure failed");
			else
				state |= S_RECONFIG;
			break;
		default:
			warn("unknown command");
	}
}

/* Possible runlevel change commands here:
	4	switch to runlevel 4, leaving sublevels unchanged
	4-	switch to runlevel 4, clear sublevels
	4ab	switch to runlevel 4, clear sublevels, activate a and b
	+ab	activate sublevels a and b
	-ac	deactivate sublevels a and c */
static void setrunlevel(const char* p)
{
	int next = nextlevel;
	int mask = 0;
	char op = *(p++);

	if(op >= '0' && op <= '9')
		next = (next & SUBMASK) | (1 << (op - '0'));
	/* should have checked for +/- here, but it's done in parsecmd before calling setrunlevel */

	if(*p == '-') {
		/* "4-" or similar */
		next &= ~SUBMASK;
		if(*(++p))
			retwarn_("no runlevels allowed past trailing -");
	} else for(; *p; p++)
		/* "4abc", or "+abc", or "-abc" */
		if(*p >= 'a' && *p <= 'f')
			mask |= (1 << (*p - 'a' + 0xa));
		else
			retwarn_("bad runlevel %c", *p);

	switch(op) {
		default: if(mask) next &= ~SUBMASK;
		case '+': next |=  mask; break;
		case '-': next &= ~mask; break;
	}

	nextlevel = next;
}

/* Convert runlevel bitmask to readable string */
static void rlstr(char* str, int len, int mask)
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

#define MAXREPORTCMD 50
/* join argv into a single string to be reported by "telinit ?" */
static char* joincmd(char* buf, int len, char** argv)
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

/* "telinit ?" output, the list of services and their current state */
static void dumpstate(void)
{
	struct initrec *p, **pp;
	char currstr[16];
	char nextstr[16];
	char cmdbuf[MAXREPORTCMD];		// " ...\0"
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

/* Set or clear flags */
static void scflags(unsigned short* dst, unsigned short flags, int setclear)
{
	if(setclear)
		*dst |= flags;
	else
		*dst &= ~flags;
}

/* Clear timestamps, forcing immediate action for spawn() and/or stop() */
static void clearts(struct initrec* p)
{
	p->lastrun = 0;
	p->lastsig = 0;
}

static void dorestart(struct initrec* p)
{
	clearts(p);
	stop(p);
}

static void doenable(struct initrec* p, int v)
{
	clearts(p);
	scflags(&(p->rlvl), currlevel & PRIMASK, v);
	scflags(&(p->flags), P_MANUAL, v);
}

static void dopause(struct initrec* p, int v)
{
	if(p->pid <= 0)
		retwarn_("%s is not running", p->name);
	if(kill(p->pid, v ? SIGSTOP : SIGCONT))
		retwarn_("%s[%i]: kill failed: %e", p->name, p->pid);

	scflags(&(p->flags), P_SIGSTOP, v);
}

static void dohup(struct initrec* p, int orstart)
{
	if(p->pid <= 0) {
		if(orstart) {
			p->lastrun = 0;
			p->rlvl  |=  (currlevel & PRIMASK);
			p->flags |=  P_MANUAL;
		} else {
			warn("%s is not running", p->name);
		}
	} else {
		kill(SIGHUP, p->pid);
	}
}
