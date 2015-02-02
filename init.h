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
#define C_DOF		(1<<3)		/* disable on failure */
#define C_ROFa		(1<<4)		/* runlevel change on failure */
#define C_ROFb		(1<<5)		/* (a/b combination determines target runlevel) */
#define C_HUSH		(1<<6)		/* do not warn about this entry */
/* C_* flags are only set in configure(),
   P_* flags may be changed at runtime */
#define P_MANUAL	(1<<8)		/* process has been disabled manually */
#define P_FAILED	(1<<9)		/* process has been disabled manually */
#define P_SIGSTOP	(1<<10)		/* SIGSTOP has been sent */
#define P_SIGTERM	(1<<11)		/* SIGTERM (or SIGABRT) has been sent to this process */
#define P_SIGKILL	(1<<12)		/* SIGKILL has been sent to this process */
#define P_ZOMBIE	(1<<13)		/* process failed to die after SIGKILL */

/* Sublevels mask (struct initrec.rlvls) */
#define PRIMASK		0x03FF		/* ------9876543210 */
#define SUBMASK		0xFC00		/* FEDCBA---------- */

/* some handy abbreviations */
#define retwarn(r, ...) return (warn(__VA_ARGS__), r)
#define retwarn_(...) { warn(__VA_ARGS__); return; }
#define gotowarn(r, ...) { warn(__VA_ARGS__); goto r; }

#define weak __attribute__((weak))
#define global

/* Each initrec represents a single process to be spawned.
   Initrecs are kept in an argv-style structure in struct config.inittab */
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
	unsigned short time_to_restart;
	unsigned short time_to_SIGKILL;	/* after sending SIGTERM, for processes refusing to die */
	unsigned short time_to_skip;	/* after sending SIGKILL */
};

/* Diagnostics; note this may go to syslog. */
int warn(const char* fmt, ...);
