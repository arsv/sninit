# Running this Makefile is *NOT* a part of normal build process!
# The library is built using top-level Makefile
# This one is only for re-generating arch-specific syscall files.

direct = execve kill getpid chroot chdir setitimer
direct += close read write fstat ioctl getdents64
direct += munmap clock_gettime nanosleep
direct += socket bind connect listen accept sendmsg recvmsg shutdown
direct += prctl setresgid setresuid setsid getegid geteuid setrlimit
direct += setpgid setpriority getpriority setfsuid setfsgid umask
direct += mremap setsockopt getuid

directs = $(patsubst %,%.s,$(direct))
directr = $(patsubst %,x-%,$(direct))

# for these, some argument mangling is required
custom = fork waitpid reboot ppoll dup3 send reboot mmap sigaction

# for quick syntax check
AS = aarch64-linux-gnu-gcc -c

directs: $(directs)
$(patsubst %,%.s,$(direct)): %.s:
	./_syscall.sh $* > $@

clean: $(patsubst %.s, x-%, $(wildcard $(directs)))
	rm -f *.o
$(directr): x-%:
	@if grep -q "# auto-generated" $*.s; then true; else echo "$*.s: not generated"; false; fi
	rm -f $*.s

.s.o:
	$(AS) $<
