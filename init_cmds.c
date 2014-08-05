#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <linux/reboot.h>
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

static inline void setrunlevel(const char* cmd);
static inline void dumpstate(void);
static inline void paused(struct initrec* p, int v);
static inline void uphup(struct initrec* p, int v);

/* cmd here is what telinit sent to initctl.
   The actual command is always cmd[0], while cmd[1:] is (optional) argument.
   Examples:
   	"q"		reconfigure
   	"=9"		switch to runlevel 9
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
		case 'r':
		case 'd': case 'e':
		case 'u': case 'w':
		case 'h': case 'k':
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
		case 'H': nextlevel = 0; rbcode = LINUX_REBOOT_CMD_HALT;      break;
		case 'P': nextlevel = 0; rbcode = LINUX_REBOOT_CMD_POWER_OFF; break;
		case 'R': nextlevel = 0; rbcode = LINUX_REBOOT_CMD_RESTART;   break;
		/* process ops */
		case 'r': stop(p); break;
		case 'p': paused(p, 1); break;
		case 'w': paused(p, 0); break;
		case 'd': p->rlvl &= ~(currlevel & PRIMASK); p->flags &= ~P_MANUAL; break;
		case 'e': p->rlvl |=  (currlevel & PRIMASK); p->flags |=  P_MANUAL; break;
		case 'h': uphup(p, 0); break;
		case 'u': uphup(p, 1); break;
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
static inline void setrunlevel(const char* p)
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

static inline void setsublevel(int add, char* arg)
{
	char c;
	int mask;

	while((c = *(arg++)))
		if(c >= 'a' && c <= 'f') {
			mask = (1 << (c - 'a' + 0xa));
			nextlevel = (add ? nextlevel | mask : nextlevel & ~mask);
		} else
			warn("bad sublevel %c", c);
}

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
/* join argv into a single string to be reported by telinit ? */
static char* makecmd(char* buf, int len, char** argv)
{
	char** arg;
	int arglen;
	int cpylen;
	char* ret = buf;

	len -= 5;
	for(arg = argv; *arg && len; arg++) {
		arglen = strlen(*arg);
		strncpy(buf, *arg, cpylen = (arglen <= len ? arglen : len));
		buf += cpylen;
		len -= cpylen;
		if(!len) break;
		*(buf++) = ' ';
		len--;
	} if(*arg) {
		strncpy(buf, " ...", cpylen = 4);
		buf += cpylen;
	}
	*buf = '\0';

	return ret;
}

/* "telinit ?" output, the list of services and their current state */
static void dumpstate(void)
{
	struct initrec *p, **pp;
	char currstr[16];
	char nextstr[16];
	char cmdbuf[MAXREPORTCMD+5];		// " ...\0"
	char* reportcmd;
	
	rlstr(currstr, 16, currlevel);
	rlstr(nextstr, 16, nextlevel);
	if(currlevel == nextlevel)
		warn("Runlevel %s", currstr);
	else
		warn("Switching %s to %s", currstr, nextstr);

	for(pp = cfg->inittab; (p = *pp); pp++) {
		if(p->flags & (C_ONCE | C_WAIT))
			if(currlevel == nextlevel)
				continue;
		reportcmd = p->name[0] ? p->name : makecmd(cmdbuf, sizeof(cmdbuf), p->argv);
		if(p->pid > 0)
			warn("%i\t%s", p->pid, reportcmd);
		else
			warn("%s\t%s", "-", reportcmd);
	}
}

static inline void paused(struct initrec* p, int v)
{
	if(p->pid <= 0)
		retwarn_("%s is not running", p->name);
	if(kill(p->pid, v ? SIGSTOP : SIGCONT))
		retwarn_("%s[%i]: kill failed: %e", p->name, p->pid);

	p->flags = v ? (p->flags | P_SIGSTOP) : (p->flags & ~P_SIGSTOP);
}

static inline void uphup(struct initrec* p, int v)
{
	if(v && p->pid <= 0) {
		p->rlvl  |=  (currlevel & PRIMASK);
		p->flags |=  P_MANUAL;
	} else if(p->pid <= 0) {
		warn("%s is not running", p->name);
	} else {
		kill(SIGHUP, p->pid);
	}
}
