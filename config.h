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

/* Directory to put process logs to, for processes with "log" flag.
   See man/inittab.5 */
#define LOGDIR "/var/log"

/* Timezone data, only used by sys_time_tz.c */
#define LOCALTIME "/etc/localtime"

/* Default runlevel. May include sublevels. */
#define INITDEFAULT (1 << 3)

/* Slippery runlevels, default value unless something else is declared in inittab. */
#define SLIPPERY ((1 << 7) | (1 << 8) | (1 << 9))

/* telinit command buffer size, also the maximum allowed command size. */
/* init will reject commands longer than this. */
#define CMDBUF 100

/* initrec name length (SysVinit standard value is 4+1) */
#define NAMELEN 8

/* Memory allocation granularity, for mmap(2). Should be something close to OS page size */
#define IRALLOC 2048

/* File size limit for inittab and (non-script) service files. */
/* Files larger than this will be loudly rejected */
#define MAXFILE 0x10000 /* 64KB */

/* Buffer size for getdents(2), when reading initdir */
#define DENTBUFSIZE 1024

/* Maximum full file name for service files; only affects initdirs */
#define FULLNAMEMAX 256

/* Clock offset to make sure boot happens at some point T > 0 */
/* See comments around setpasstime for why this is necessary. */
#define BOOTCLOCKOFFSET 0xFFFF
