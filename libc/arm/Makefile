# Running this Makefile is *NOT* a part of normal build process!
# The library is built using top-level Makefile
# This one is only for re-generating arch-specific syscall files.

# Args 5 and 6 are register for syscalls but stacked for functions
# To avoid doing extra work, only include relevant code for calls
# which actually use those args.
direct4 = fork execve kill getpid time
direct4 += open close read write dup2 fstat ioctl getdents64 openat stat
direct4 += munmap clock_gettime
direct4 += socket bind connect listen accept sendmsg recvmsg shutdown
direct4 += prctl setresgid setresuid capset setsid getegid geteuid setrlimit
direct4 += send sigaction sigprocmask
direct5 = mremap setsockopt
direct6 = sendto

direct = $(direct4) $(direct5) $(direct6)
directs = $(patsubst %,%.s,$(direct))
directr = $(patsubst %,x-%,$(direct))

# for these, some argument mangling is required
custom = mmap waitpid reboot ppoll

# for quick syntax check
AS = arm-none-eabi-gcc -c

directs: $(directs)
$(patsubst %,%.s,$(direct4)): %.s:
	./_syscall.sh $* > $@
$(patsubst %,%.s,$(direct5)): %.s:
	./_syscall.sh $* 5 > $@
$(patsubst %,%.s,$(direct6)): %.s:
	./_syscall.sh $* 6 > $@

clean: $(patsubst %.s, x-%, $(wildcard $(directs)))
	rm -f *.o
$(directr): x-%:
	@if grep -q "# auto-generated" $*.s; then true; else echo "$*.s: not generated"; false; fi
	rm -f $*.s

.s.o:
	$(AS) $<
