#include <sys/types.h>
#include <sys/time.h>
#include "config.h"

/* Global state (int state) */
#define S_SIGCHLD	(1<<0)		/* SIGCHLD was received, got to call wait() */
#define S_INITCTL	(1<<1)		/* telinit connection on initctlfd is waiting to be accept()ed */
#define S_RECONFIG	(1<<2)		/* new configuration is ready in newblock */
#define S_WAITING	(1<<3)		/* waiting for some process (w- or o-type) to finish */

/* Per-process flags (struct initrec.flags) */
#define C_INVERT	(1<<0)		/* invert runlevel mask */
#define C_ONCE		(1<<1)		/* o-type entry; run once, do not restart */
#define C_WAIT		(1<<2)		/* w-type entry; wait before starting, wait until finished */
#define C_HUSH		(1<<3)		/* do not warn about this entry */
#define C_FAST		(1<<4)		/* use time_to_restart instead of minimum_runtime */
#define C_KILL		(1<<5)		/* do not bother with SIGTERM, just shoot the damn thing */
#define C_SHELL		(1<<6)		/* sh -c is used to run this command */
/* C_* flags are only set in configure(),
   P_* flags may be changed at runtime */
#define P_MANUAL	(1<<8)		/* entry has been stopped manually */
#define P_FAILED	(1<<9)		/* entry was respawning too fast  */
#define P_SIGSTOP	(1<<10)		/* SIGSTOP has been sent */
#define P_SIGTERM	(1<<11)		/* SIGTERM (or SIGABRT) has been sent to this process */
#define P_SIGKILL	(1<<12)		/* SIGKILL has been sent to this process */
#define P_WAS_OK	(1<<13)		/* previous run finished well */

/* Sublevels mask (struct initrec.rlvls) */
#define PRIMASK		0x03FF		/* ------9876543210 */
#define SUBMASK		0xFC00		/* FEDCBA---------- */

/* configure() arguments */
#define NONSTRICT	0
#define STRICT		1

/* some handy abbreviations */
#define retwarn(r, ...) return (warn(__VA_ARGS__), r)
#define retwarn_(...) { warn(__VA_ARGS__); return; }
#define gotowarn(r, ...) { warn(__VA_ARGS__); goto r; }

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
	int initnum;			/* inittab element count */
	struct initrec** inittab;	/* array of pointers, like env */
	char** env;			/* to be passed to execve in children */
};

/* Diagnostics; note this may go to syslog. */
int warn(const char* fmt, ...);
