/* Logging directory */
#define LOGDIR "/var/log"

/* CGroups mount dir */
#define CGBASE "/sys/fs/cgroup"
/* Now technically this is incorrect, cgroups can be mounted at any location.
   libcgroup uses /etc/cgconfig.conf to find the path, a C-syntax file with
   nested blocks for no apparent reason. Parsing that in a small utility does
   not look like a good idea.
   However, for a given system, it's probably safe to assume the path is fixed. */

/* Size limit for mmaped /etc/passwd and /etc/group */
#define MAXFILELEN 65536
/* Maximum number of secondary groups */
#define MAXGROUPS 20
