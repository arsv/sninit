# Target architecture, for bundled libc only
ARCH := x86_64

# Building
CC := gcc
AS := $(CC) -c
AR := ar
CFLAGS := -Wall -g
ASFLAGS :=
LDFLAGS :=
LIBS :=

# Installation directories. Check config.h for runtime paths.
sbindir := /sbin
man5dir := /usr/share/man/man5
man8dir := /usr/share/man/man8
# Installation basename prefix (as in {,s,sn}init)
s := 

# Built-in inittab
builtin := 
# Needed for $(builtin) parse tool
HOSTCC :=

# Init block configuration: {conf|null} {tz|notz|libcts|nots} [sys|err] [dents]
#
# runtime configuration
#	conf	configurable init (can/does read /etc/inittab)
#	null	null configuration, only builtin inittab
# timestamps for syslog
#	tz	parse /etc/localtime with bundled routines
#	notz	do parse /etc/localtime, timestamps in UTC
#	nots	do not put timestamps at all, rely on syslogd to add them
#	libcts	use libc strftime to make timestamps
# bundled routines (read sys_* files to see what does that mean)
#	sys	vsnprintf() and strerror()
#	err	strerror() (implied by sys)
#	dents	getdents64() (only needed when linking against glibc)
#	ppoll	ppoll via syscall() (only needed with unpatched dietlibc)
initblocks := conf tz sys

# Set up bundled libc stuff
# Note $/ here expands to the top-level directory
ifneq ($(ARCH),)
override CFLAGS += -nostdinc -I$/libc/include -I$/libc/$(ARCH)
override LDFLAGS += -nostdlib
override LIBS := $/libc.a $(LIBS) -lgcc
endif
