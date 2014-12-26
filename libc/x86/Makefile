# Running this Makefile is *NOT* a part of normal build process!
# The library is built using top-level Makefile
# This one is only for re-generating arch-specific syscall files.

# syscalls that do no require any additional setup
direct = fork execve kill getpid time waitpid
direct += open close read write dup2 fstat ioctl getdents64 openat stat
direct += mremap munmap clock_gettime
direct += prctl setresgid setresuid capset setsid getegid geteuid setrlimit

socket = socket setsockopt bind connect listen accept sendto sendmsg recvmsg
socket += shutdown send

directs = $(patsubst %,%.s,$(direct))
sockets = $(patsubst %,%.s,$(socket))

gens = $(directs) $(sockets)
genr = $(patsubst %.s,x-%,$(gens))

# for these, some argument mangling is required
custom = ppoll reboot sigaction sigprocmask time socketcall mmap

# for quick syntax check
AS = gcc -m32 -march=i686 -c

all: $(gens)

directs: $(directs)
$(directs): %.s:
	./_syscall.sh $* > $@

sockets: $(sockets)
$(sockets): %.s:
	./_syssock.sh $* > $@

clean: $(patsubst %.s,x-%,$(wildcard $(gens)))
	rm -f *.o
$(genr): x-%:
	@if grep -q "# auto-generated" $*.s; then true; else echo "$*.s: not generated"; false; fi
	rm -f $*.s

.s.o:
	$(AS) $<
