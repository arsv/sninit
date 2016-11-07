# Target architecture for bundled libc. Leave empty to use system libc.
ARCH := x86_64

# Building
CC := gcc
AS := $(CC) -c
AR := ar
STRIP := strip
CFLAGS := -Os -fno-asynchronous-unwind-tables -mno-sse
ASFLAGS := 
LDFLAGS := 
LIBS := -lgcc
# for compatibility with GNU make built-in rules
LDLIBS = $(LIBS)
# Emulator to run tests on non-native platform
RUN := 

# Installation directories; check config.h for runtime paths
sbindir := /sbin
man5dir := /usr/share/man/man5
man8dir := /usr/share/man/man8
# Installation basename prefix (as in {,s,sn}init)
s := 

# Built-in inittab
builtin := 
# Needed for $(builtin) parse tool
HOSTCC := 

# Init block configuration: {conf|null} [sys|err] [dents,ppoll,unshare]
#
# runtime configuration
#	conf	configurable init (can/does read /etc/inittab)
#	null	null configuration, only builtin inittab
# bundled routines (read sys_* files to see what that means)
#	sys	vsnprintf() and strerror()
#	err	strerror() (implied by sys)
#	dents	getdents64() (only needed when linking against glibc)
#	ppoll	ppoll via syscall() (only needed with unpatched dietlibc)
#	unshare	unsahre via syscall (only needed with unpatched dietlibc)
initblocks := conf sys

# Extra targets to build on "make all". Only used for sbin atm.
extra := 

# Set up bundled libc stuff
# Note $/ here expands to the top-level directory
ifneq ($(ARCH),)
override CFLAGS += -nostdinc -I$/libc -I$/libc/include -I$/libc/arch/$(ARCH)
override LDFLAGS += -nostdlib
override LIBS := $(LIBS) $/libc.a
endif

# On arm, there is a circular dependency between libc.a and libgcc.a
# which ld can not resolve on its own; the offending symbols are
# raise(3) from libc and __aeabi_idiv{,mod} from libgcc.
# What the following piece does is making sure there is libc after
# libgcc *and* libgcc after libc, which kinda tricks ld into linking
# everything together. Icky, considering -z muldefs above.
ifeq ($(ARCH),arm)
override LIBS := $(LIBS) $(LIBS)
endif
