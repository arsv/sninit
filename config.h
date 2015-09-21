/* init control socket address (AF_UNIX), for init-telinit communication. */
/* @ at the start will be replaced with \0, see unix(7) */
/* The only common alternative here is /dev/initctl */
#define INITCTL "@initctl"

/* The main configuration file aka inittab. */
/* MAY be a relative path, which will PROBABLY end up with / for cwd */
#define INITTAB "/etc/inittab"
/* Initdir, for service files. Leave undefined to disable initdir parsing */
#define INITDIR "/etc/initdir"

/* Syslog socket for init's own messages. Do not change, it's a standard
   value hard-coded in several syslog(3) implementations. */
#define SYSLOG "/dev/log"

/* Directory to put process logs to. This is for run, not for init. */
#define LOGDIR "/var/log"

/* Timezone data, only used by sys_time_tz.c */
#define LOCALTIME "/etc/localtime"

/* initrec name length (SysVinit standard value is 8+1) */
/* makes sense to keep it a multiple of 4 */
#define NAMELEN 16

/* Include some tweaks for non-MMU systems.
   Beware: enabling this may work on MMU systems, not enabling this
   on non-MMU systems may build well and fail at runtime. */
//#define NOMMU

/* Development mode: do not (attempt to) reboot,
   and do not assume uid = euid = 0 */
//#define DEVMODE

/* Timing constants, in seconds */

/* minimum time between successive attempts to spawn the same entry */
#define TIME_TO_RESTART 2
/* entries are allowed to do post-SIGTERM cleanup for this long */
#define TIME_TO_SIGKILL 8
/* an entry that fails to die for this long after SIGKILL is skipped */
#define TIME_TO_SKIP    2
/* regular daemons must run for at least this long
 * to be considered non-failing */
#define MINIMUM_RUNTIME 8
/* maximum time single telinit session may take */
#define INITCTL_TIMEOUT 2

/* If you change the runlevels below, be ready to rewrite manual pages! */

/* Default runlevel. May include sublevels. */
#define INITDEFAULT (1 << 3)

/* Slippery runlevels. See doc/slippery.txt for explanation. */
#define SLIPPERY ((1 << 7) | (1 << 8) | (1 << 9))

/* Default mode for initdir entries */
/* (this is a string parsed just like the 2nd inittab field) */
#define SRDEFAULT "S3+"

/* Fallback runlevels, see C_ROFa and C_ROFb from init.h */
#define FALLBACK1 1
#define FALLBACK2 2
#define FALLBACK3 3

/* Do not change the following values unless you know what you are doing. */
/* Wide range of values will work without affecting anything visible to the user. */

/* telinit command buffer size, also the maximum allowed command size. */
/* init will reject commands longer than this. */
#define CMDBUF 100

/* Memory allocation granularity, for mmap(2). Should be something close to OS page size */
#define IRALLOC 4096

/* File size limit for inittab and (non-script) service files. */
/* Files larger than this will be loudly rejected */
#define MAXFILE 0x10000 /* 64KB */

/* Buffer size for getdents(2), when reading initdir */
#define DENTBUFSIZE 1024

/* Maximum full file name for service files; only affects initdirs */
/* Should be at least strlen(INITDIR) + NAMELEN + 2 */
#define FULLNAMEMAX 64

/* Clock offset to make sure boot happens at some point T > 0 */
/* See comments around setpasstime for why this is necessary. */
#define BOOTCLOCKOFFSET 0xFFFF

/* Maximum command length in telinit ? output */
#define MAXREPORTCMD 50
