#include <string.h>
#include <stdlib.h>
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

static inline void setsublevel(int add, char* arg);
static inline void setrunlevel(char rl);
static inline void dumpstate(void);

/* cmd here is what telinit sent to initctl.
   The actual command is always cmd[0], while cmd[1:] is an (optional) argument.
   Examples:
   	"q"		reconfigure
   	"=9"		switch to runlevel 9
	"dhttpd"	disable (stop) service named "httpd"

   warn() sends messages back to telinit */

void parsecmd(char* cmd)
{
	char* arg = cmd + 1;
	struct initrec* p = NULL;

	switch(*cmd) {
		case 'r':
		case 'd':
		case 'e':
			if(!(p = findentry(arg))) {
				warn("can't find %s in inittab", arg);
				return;
			}
	}
	switch(*cmd) {
		case '+':
		case '-':
			setsublevel(*cmd == '+', arg);
			break;
		case '=':
			setrunlevel(*arg);
			break;
		case 'r':
			stop(p);
			break;
		case 'd':
			p->flags |= C_DISABLED;
			break;
		case 'e':
			p->flags &= ~C_DISABLED;
			break;
		case 'q':
			if(configure(1))
				warn("!configure failed");
			else
				state |= S_RECONFIG;
			break;
		case 'H':
			rbcode = LINUX_REBOOT_CMD_HALT;
			nextlevel = 0;
			break;
		case 'P':
			rbcode = LINUX_REBOOT_CMD_POWER_OFF;
			nextlevel = 0;
			break;
		case 'R':
			rbcode = LINUX_REBOOT_CMD_RESTART;
			nextlevel = 0;
			break;
		case '?':
			dumpstate();
			break;
		default:
			warn("unknown command");
	}
}

static inline void setrunlevel(char rl)
{
	if(rl >= '0' && rl <= '9')
		nextlevel = (nextlevel & SUBMASK) | (1 << (rl - '0'));
	else
		warn("bad runlevel %c", rl);
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

#define MAXREPORTCMD 50
/* join argv into a single string to be reported by telinit ? */
char* makecmd(char* buf, int len, char** argv)
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
static inline void dumpstate(void)
{
	struct initrec* p;
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

	for(p = cfg->inittab; p; p = p->next) {
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
