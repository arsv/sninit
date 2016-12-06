#include "../init.h"
#include "../init_conf.h"
#include "_test.h"

struct nblock newblock;

extern int shellneeded(const char* cmd);

#define shell(s) \
	ASSERT(shellneeded(s))
#define notshell(s) \
	ASSERT(!shellneeded(s))

int main(void)
{
	notshell("/sbin/httpd");
	notshell("/sbin/httpd -n ");
	notshell("/usr/bin/vsftpd /etc/vsftpd.conf -obackground=NO");

	/* plain echo should be a shell builtin, even with redirection */
	shell("echo Initializing network");
	shell("echo mem > /sys/power/state");
	/* standalone echo, definitely not a shell builtin */
	shell("/bin/echo \"Initializing network\"");
	shell("/bin/echo mem > /sys/power/state");
	/* explicit exec in otherwise non-shell entry */
	shell("exec /sbin/ntpd");
	/* semicolon in a non-shell command, leaves sh process waiting */
	shell("/sbin/httpd -n ;");

	/* quotes with semicolons inside */
	shell("/usr/sbin/nginx -g 'daemon off; master_process on;'");
	/* bad example since this should have exec */

	/* XXX: asterisk is not enough to force shell */
	notshell("/usr/sbin/thd /dev/input/event*");
	shell("exec /usr/libexec/nfc/neard -d '*'");	/* <- these two */
	notshell("/usr/libexec/nfc/neard -d *");	/* <- are equivalent */

	return 0;
}
