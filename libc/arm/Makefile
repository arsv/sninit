# Running this Makefile is *NOT* a part of normal build process!
# The library is built using top-level Makefile
# This one is only for re-generating arch-specific syscall files.

# Args 5 and 6 are register for syscalls but stacked for functions
# To avoid doing extra work, only include relevant code for calls
# which actually use those args.
direct4 = fork vfork execve kill getpid chroot chdir setitimer
direct4 += open close read write dup2 stat fstat ioctl getdents64
direct4 += munmap clock_gettime nanosleep setfsuid setfsgid umask
direct4 += socket bind connect listen accept shutdown
direct4 += setresgid setresuid setsid setrlimit
direct4 += send sigaction sigprocmask setpriority
direct4 += getuid unshare
direct5 = mremap getsockopt

direct = $(direct4) $(direct5)
directs = $(patsubst %,%.s,$(direct))
directr = $(patsubst %,x-%,$(direct))

# for these, some argument mangling is required
custom = mmap waitpid reboot ppoll

# for quick syntax check
AS = arm-linux-gnueabi-gcc -c

directs: $(directs)
$(patsubst %,%.s,$(direct4)): %.s:
	./_syscall.sh $* > $@
$(patsubst %,%.s,$(direct5)): %.s:
	./_syscall.sh $* 5 > $@

clean: $(patsubst %.s, x-%, $(wildcard $(directs)))
	rm -f *.o
$(directr): x-%:
	@if grep -q "# auto-generated" $*.s; then true; else echo "$*.s: not generated"; false; fi
	rm -f $*.s

.s.o:
	$(AS) $<
