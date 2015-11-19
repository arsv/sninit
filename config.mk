# Target architecture for bundled libc. Leave empty to use system libc.
ARCH :=

# Building
CC := cc
AS := $(CC) -c
AR := ar
STRIP := strip
CFLAGS := -Os
ASFLAGS :=
LDFLAGS :=
LIBS :=
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
HOSTCC := gcc

# Init block configuration: {conf|null} {tz|notz|libcts|nots} [sys|err] [dents,ppoll]
#
# runtime configuration
#	conf	configurable init (can/does read /etc/inittab)
#	null	null configuration, only builtin inittab
# timestamps for syslog
#	tz	parse /etc/localtime with bundled routines
#	notz	do parse /etc/localtime, timestamps in UTC
#	nots	do not put timestamps at all, rely on syslogd to add them
#	libcts	use libc strftime to make timestamps
# bundled routines (read sys_* files to see what that means)
#	sys	vsnprintf() and strerror()
#	err	strerror() (implied by sys)
#	dents	getdents64() (only needed when linking against glibc)
#	ppoll	ppoll via syscall() (only needed with unpatched dietlibc)
initblocks := conf sys nots dents

# Extra targets to build on "make all". Only used for sbin atm.
extra :=

# Set up bundled libc stuff
# Note $/ here expands to the top-level directory
ifneq ($(ARCH),)
override CFLAGS += -nostdinc -I$/libc/include -I$/libc/$(ARCH)
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
