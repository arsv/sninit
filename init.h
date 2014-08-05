#include <sys/types.h>
#include <sys/time.h>
#include "config.h"

/* Global state (int state) */
#define S_PID1		(1<<0)		/* running a process #1 */
#define S_SIGCHLD	(1<<1)		/* SIGCHLD was received, got to call wait() */
#define S_INITCTL	(1<<2)		/* telinit connection on initctlfd is waiting to be accept()ed */
#define S_RECONFIG	(1<<3)		/* new configuration is ready in newblock */
#define S_WAITING	(1<<4)		/* waiting for some process (w- or o-type) to finish */

/* Per-process flags (struct initrec.flags) */
#define C_ONCE		(1<<0)		/* o-type entry; run once, do not restart */
#define C_WAIT		(1<<1)		/* w-type entry; wait before starting, wait until finished */
#define C_USEABRT	(1<<2)		/* use SIGABRT instead of SIGINT when stopping process */
#define C_NULL		(1<<3)		/* redirect stdout & stderr to /dev/null */
#define C_LOG		(1<<4)		/* redirect stdout & stderr to /var/log/(name) */
#define C_SH		(1<<5)		/* argv = [ /bin/sh -c "command" ]; for reference only, exec(argv) still works */
#define C_TTY		(1<<6)		/* setup controlling tty for this process (aka interactive) */

#define P_MANUAL	(1<<8)		/* process has been enabled manually */
#define P_SIGSTOP	(1<<9)		/* SIGSTOP has been sent */
#define P_SIGTERM	(1<<10)		/* SIGTERM (or SIGABRT) has been sent to this process */
#define P_SIGKILL	(1<<11)		/* SIGKILL has been sent to this process */
#define P_ZOMBIE	(1<<12)		/* process failed to die after SIGKILL */

/* Sublevels mask (struct initrec.rlvls) */
#define PRIMASK		0x03FF		/* ------9876543210 */
#define SUBMASK		0xFC00		/* FEDCBA---------- */

/* some handy abbreviations */
#define retwarn(r, ...) return (warn(__VA_ARGS__), r)
#define retwarn_(...) { warn(__VA_ARGS__); return; }
#define gotowarn(r, ...) { warn(__VA_ARGS__); goto r; }

#define weak __attribute__((weak))
#define global

/* Initrecs are kept in a double-linked list.
   Why double: see initpass(), specifically the comments on kill/spawn order.
   Why list: it would make just as much sense making them into an argv-style array,
   i.e. pointers followed by structs, but that would require scratching the pointers
   somehow, complicating configure() code.
   Self-contained initrecs can be placed directly in newblock. */
struct initrec {
	char name[NAMELEN];

	unsigned short rlvl;	// bitmask: [FEDCBA9876543210]
	unsigned short flags;	// per-process flags, see S_ constants above

	int pid;	/* >0: the process is running
			   =0: the process hasn't been started
			   -1: the process died (set after waitpid() call) */
	time_t lastrun;
	time_t lastsig;

	char* argv[];
};

/* Primary configuration struct.
   Typically found at the start of cfgblock, but the one produced by statictab
   has no corresponding memblock and resides in .data instead. */
struct config {
	int slippery;		/* runlevel bitmask; do not remain in runlevel,
				   revert immediately */

	int initnum;			/* inittab element count */
	struct initrec** inittab;	/* array of pointers, like env */
	char** env;			/* to be passed to execve in children */

	/* time_* values are in seconds */
	int time_to_restart;
	int time_to_SIGKILL;	/* after sending SIGTERM, for processes refusing to die */
	int time_to_skip;	/* after sending SIGKILL */

	char* logdir;
};

/* Diagnostics; note this may go to syslog. */
int warn(const char* fmt, ...);
