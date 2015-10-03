# Running this Makefile is *NOT* a part of normal build process!
# The library is built using top-level Makefile
# This one is only for re-generating arch-specific syscall files.

direct = fork execve kill getpid chroot chdir setitimer
direct += open close read write dup2 fstat ioctl getdents64 openat stat
direct += mmap mremap munmap clock_gettime nanosleep setpriority getpriority
direct += socket bind connect listen accept sendto sendmsg recvmsg shutdown
direct += prctl setresgid setresuid setsid getegid geteuid setrlimit umask
direct += send setsockopt sigaction sigprocmask waitpid setpgid setfsuid
direct += setfsgid getuid unshare

directs = $(patsubst %,%.s,$(direct))
directr = $(patsubst %,x-%,$(direct))

# for these, some argument mangling is required
custom = ppoll reboot

# for quick syntax check
AS = mipsel-linux-gnu-gcc -c

directs: $(directs)
$(directs): %.s:
	./_syscall.sh $* > $@

clean: $(patsubst %.s, x-%, $(wildcard $(directs)))
	rm -f *.o
$(directr): x-%:
	@if grep -q "# auto-generated" $*.s; then true; else echo "$*.s: not generated"; false; fi
	rm -f $*.s

.s.o:
	$(AS) $<
