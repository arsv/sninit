/* runcap is a companion utility to sninit, in the sense it does what
   could have been implemented in sninit (but wasn't).
   See doc/limits.txt for discussion of the problem.

   Yet even with background like this, runcap is completely independent
   from sninit. It could easily be packaged and distributed as a stanalone utility.

   This perfect separation of functions was actually one of the main reasons
   to make it standalone, vs. having all this built into init. */

#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <linux/capability.h>
#include <sys/capability.h>
#include <string.h>
#include <errno.h>

/* Size limit for mmaped /etc/passwd and /etc/group */
#define MAXFILELEN 65536
/* Maximum number of secondary groups */
#define MAXGROUPS 20

#define ERRNO ((void*)-1)

/* Now technically this is incorrect, cgroups can be mounted at any location.
   libcgroup uses /etc/cgconfig.conf to find the path, a C-syntax file with
   nested blocks for no apparent reason. Parsing that in a small utility does
   not look like a good idea.
   However, for a given system, it's probably safe to assume the path is fixed. */
#define CGBASE "/sys/fs/cgroup"

/* Stored process limitations: uid/gids and capabilities */
/* Cgroups and ulimits are applied immediately, no need to store those */
uid_t uid = -1;
gid_t gid = -1;
/* Secondary groups */
int gidn = 0;
gid_t gids[MAXGROUPS];
/* Capabilities */
uint32_t caps[VFS_CAP_U32] = { 0 };
/* ulimits and cgroups are applied immediately, so no variables for them */

/* Files for looking up uids and gids by name */
struct rcfile {
	const char* name;
	char* buf;
	size_t len;
	char* ls;		/* line start */
	char* le;		/* line end */
} passwd = {
	.name = "/etc/passwd"
}, groups = {
	.name = "/etc/group"
};

/* ulimit keys, closely following bash(1) ulimit command */
/* note T is used for RTTIME here; in bash, it's apparently NPROC */
struct limitdesc {
	char key;
	int resource;
} ulimits[] = {
	{ 'a', RLIMIT_AS },
	{ 'c', RLIMIT_CORE },
	{ 't', RLIMIT_CPU },
	{ 'd', RLIMIT_DATA },
	{ 'f', RLIMIT_FSIZE },
	{ 'x', RLIMIT_LOCKS },
	{ 'l', RLIMIT_MEMLOCK },
	{ 'q', RLIMIT_MSGQUEUE },
	{ 'e', RLIMIT_NICE },
	{ 'n', RLIMIT_NOFILE },
	{ 'p', RLIMIT_NPROC },
	{ 'm', RLIMIT_RSS },
	{ 'E', RLIMIT_RTPRIO },
	{ 'T', RLIMIT_RTTIME },
	{ 'i', RLIMIT_SIGPENDING },
	{ 's', RLIMIT_STACK },
	{ '\0' }
};

/* Using shorter names for capabilities here, things like cap_net_bind_service
   are way too long to type and make no sense anyway. */
struct capdesc {
	const char* key;
	int cap;
} capabilities[] = {
	{ "chown", 		CAP_CHOWN },
	{ "dac-override", 	CAP_DAC_OVERRIDE },
	{ "dac-read-search",	CAP_DAC_READ_SEARCH },
	{ "fowner",		CAP_FOWNER },
	{ "fsetid",		CAP_FSETID },
	{ "kill",		CAP_KILL },
	{ "setgid",		CAP_SETGID },
	{ "setuid",		CAP_SETUID },
	{ "setpcap",		CAP_SETPCAP },
	{ "immutable",		CAP_LINUX_IMMUTABLE },
	{ "bind",		CAP_NET_BIND_SERVICE },
	{ "broadcast",		CAP_NET_BROADCAST },
	{ "netadmin",		CAP_NET_ADMIN },
	{ "netraw",		CAP_NET_RAW },
	{ "ipclock",		CAP_IPC_LOCK },
	{ "ipcowner",		CAP_IPC_OWNER },
	{ "module",		CAP_SYS_MODULE },
	{ "rawio",		CAP_SYS_RAWIO },
	{ "chroot",		CAP_SYS_CHROOT },
	{ "ptrace",		CAP_SYS_PTRACE },
	{ "pacct",		CAP_SYS_PACCT },
	{ "sysadmin",		CAP_SYS_ADMIN },
	{ "boot",		CAP_SYS_BOOT },
	{ "nice",		CAP_SYS_NICE },
	{ "resource",		CAP_SYS_RESOURCE },
	{ "time",		CAP_SYS_TIME },
	{ "ttyconfig",		CAP_SYS_TTY_CONFIG },
	{ "mknod",		CAP_MKNOD },
	{ "lease",		CAP_LEASE },
	{ "auditrewrite",	CAP_AUDIT_WRITE },
	{ "auditcontrol",	CAP_AUDIT_CONTROL },
	{ "setfcap",		CAP_SETFCAP },
	{ "macoverride",	CAP_MAC_OVERRIDE },
	{ "macadmin",		CAP_MAC_ADMIN },
	{ "syslog",		CAP_SYSLOG },
	{ "wakealarm",		CAP_WAKE_ALARM },
	{ "blocksuspend",	CAP_BLOCK_SUSPEND },
	{ NULL }
};

static void die(const char* msg, const char* arg, const char* err) __attribute__((noreturn));

static void parsecaps(char* caps);

static void adduser(char* p);
static void addgroup(char* p);
static void addsgroup(char* p);
static void setlimit(char* p);
static void setcg(char* p);
static void addcap(char* p);

static void applycaps();

int main(int argc, char** argv)
{
	if(argc < 3) 
		die("Usage: runcap [capabilities] command arg arg arg ...", NULL, NULL);

	parsecaps(argv[1]);
	applycaps();

	execvp(argv[2], argv + 2);
	die("Can't exec ", argv[2], ERRNO);
}

/* user:group@subgroup1,subgroup2.n100s65M.T10+bind */
static void parsecaps(char* caps)
{
	char* p = caps;
	char* q;
	char qs;
	char mode = '~';

	while(*p) {
		switch(*p) {
			case ':':
			case '@':
			case '.':
			case '+': 
			case '%': mode = *p;
			case ',': p++;
		}
		for(q = p; *q && !strchr(":%@.+,", *q); q++);
		/* user@group,subgroup1... */
		/* p ---^    ^--- q        */
		qs = *q; *q = '\0';
		/* user@group₀subgroup1... */
		switch(mode) {
			case '~': adduser(p); break;
			case ':': addgroup(p); break;
			case '@': addsgroup(p); break;
			case '+': addcap(p); break;
			case '.': setlimit(p); break;
			case '%': setcg(p); break;
		};
		*q = qs; p = q;
		/* user@group,subgroup1... */
		/*           ^--- p        */
	};
}

/* rc* to avoid name clashes with init_conf_mem, keeping ctags consistent */
/* Note there's no munmap; mmaped files are dropped during exec, so why bother */
static void rcmapfile(struct rcfile* f)
{
	int fd;
	struct stat st;

	if((fd = open(f->name, O_RDONLY)) < 0)
		die("Can't open ", f->name, ERRNO);
	if(fstat(fd, &st))
		die("Can't stat ", f->name, ERRNO);
	if(st.st_size > MAXFILELEN)
		die("Can't mmap ", f->name, "the file is too large");
	if((f->buf = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
		die("Can't mmap ", f->name, ERRNO);

	close(fd);
	f->len = st.st_size;

	f->ls = NULL;
	f->le = NULL;
};

static void rcrewind(struct rcfile* f)
{
	f->ls = NULL;
	f->le = NULL;
};

static char* rcnextline(struct rcfile* f)
{
	if(f->le)
		*(f->le) = '\n';
	f->ls = (f->le ? f->le + 1 : f->buf);
	if(f->ls >= f->buf + f->len)
		return NULL;
	
	for(f->le = f->ls; *(f->le) && *(f->le) != '\n'; f->le++);
	*(f->le) = '\0';

	return f->ls;
};

/* This can only be called once, so it's ok to munmap passwd immediately */
static void adduser(char* user)
{
	char* l;
	char* name;
	char* uidstr;
	char* gidstr;

	rcmapfile(&passwd);
	while((l = rcnextline(&passwd))) {
		name = strsep(&l, ":");
		if(strcmp(name, user))
			continue;
		strsep(&l, ":");
		uidstr = strsep(&l, ":");
		gidstr = strsep(&l, ":");
		if(!uidstr || !gidstr)
			continue;
		uid = atoi(uidstr);
		gid = atoi(gidstr);
		break;
	}
};

static gid_t findgroup(char* group)
{
	char* l;
	char* grpname;
	char* gidstr;

	if(!groups.buf)
		rcmapfile(&groups);
	else
		rcrewind(&groups);

	while((l = rcnextline(&groups))) {
		grpname = strsep(&l, ":");
		if(strcmp(grpname, group))
			continue;
		strsep(&l, ":");
		gidstr = strsep(&l, ":");
		return atoi(gidstr);
	};

	die("Unknown group ", group, NULL);
}

static void addgroup(char* group)
{
	gid = findgroup(group);
};

static void addsgroup(char* group)
{
	if(gidn >= MAXGROUPS)
		die("Too many groups", NULL, NULL);
	gids[gidn++] = findgroup(group);
};

/* Limits could have been stored just like uid/gids and caps... except it's not needed.
   Unlike uid/gid/cap changes which must be coordinated, limit settings are independent
   and do not prevent runcap from doing its work even if set immediately. So, why bother. */
static void setlimit(char* lim)
{
	struct limitdesc* p;
	char* hard;
	char* soft;
	struct rlimit rl;

	for(p = ulimits; p->key; p++)
		if(p->key == *lim)
			break;
	if(!p->key)
		die("Unknown limit key ", lim, NULL);

	char* q = lim + 1;
	soft = strsep(&q, "/");
	hard = q;

	rl.rlim_cur = atoi(soft);
	rl.rlim_max = hard ? atoi(hard) : rl.rlim_cur;

	if(setrlimit(p->resource, &rl))
		die("Can't set ulimit ", lim, ERRNO);
};

/* may be used by strerror, so exported */
char* ltoa(long num)
{
	static char buf[16];
	int i = sizeof(buf)-1;
	buf[i--] = '\0';
	while(num && i >= 0) {
		buf[i--] = num % 10;
		num /= 10;
	};
	return buf + i + 1;
}

static void setcg(char* cg)
{
	int cgl = strlen(CGBASE) + strlen(cg) + 15;
	char cgp[cgl];
	int fd;
	char* pid;

	strcpy(cgp, CGBASE);
	strcat(cgp, "/");
	strcat(cgp, cg);
	strcat(cgp, "/tasks");

	if((fd = open(cgp, O_WRONLY)) < 0)
		die("Can't open ", cgp, ERRNO);

	pid = ltoa(getpid());
	if(write(fd, pid, strlen(pid)) <= 0)
		die("Can't add process to ", cgp, ERRNO);
	close(fd);
}

static void addcap(char* cap)
{
	struct capdesc* p;

	for(p = capabilities; p->key; p++)
		if(!strcmp(p->key, cap))
			break;
	if(!p->key)
		die("Unknown capability ", cap, NULL);

	caps[CAP_TO_INDEX(p->cap)] |= CAP_TO_MASK(p->cap);
};

/* Ulimits, if any, have been set earlier. That doesn't affect anything here. */
static void applycaps()
{
	struct __user_cap_header_struct caphdr = {
		.version = _LINUX_CAPABILITY_VERSION_3,
		.pid = 0
	};
	struct __user_cap_data_struct capepi[VFS_CAP_U32];
	int hascaps = 0;
	int i;

	for(i = 0; i < VFS_CAP_U32; i++) {
		capepi[i].effective = caps[i];
		capepi[i].permitted = caps[i];
		capepi[i].inheritable = caps[i];
		if(caps[i]) hascaps = 1;
	}
	
	if(hascaps)
		if(prctl(PR_SET_KEEPCAPS, 1L))
			die("prctl failed", NULL, ERRNO);
	if(gid >= 0)
		if(setresgid(gid, gid, gid))
			die("setresgid failed", NULL, ERRNO);
	if(uid >= 0)
		if(setresuid(uid, uid, uid))
			die("setresuid failed", NULL, ERRNO);
	if(hascaps)
		if(capset(&caphdr, capepi))
			die("capset failed", NULL, ERRNO);
};

static char* strlncat(char* buf, char* ptr, int buflen, const char* str)
{
	int len = strlen(str);
	int avail = (buf + buflen) - ptr;
	if(len > avail) len = avail;
	memcpy(ptr, str, len);
	return ptr + len;
}

static void die(const char* msg, const char* arg, const char* err)
{
	int buflen = 1024;
	char buf[buflen+1];
	char* ptr = buf;

	ptr = strlncat(buf, ptr, buflen, msg);
	if(arg)
		ptr = strlncat(buf, ptr, buflen, arg);
	if(err == ERRNO)
		err = strerror(errno);
	if(err) {
		ptr = strlncat(buf, ptr, buflen, ": ");
		ptr = strlncat(buf, ptr, buflen, err);
	}
	*ptr = '\n';

	write(2, buf, ptr - buf + 1);
	_exit(-1);
}
