# To build sninit, check configuration variables below and run "make".
# Alternatively, you can use ./configure, which will set those variables
# to some pre-defined values.

# Do not rely on configure too much though, it is only expected to handle
# standard situations. In case you need some unusual setup (weirdly configured
# dietlibc etc), ditch configure and edit this file directly.

# Target architecture, for bundled libc only
ARCH := x86

# Building
CC := gcc -m32 -march=i686
AR := ar
CFLAGS := -Wall -g
LDFLAGS :=
LIBS := libc.a -lgcc

# Installation directories. Check config.h for runtime paths.
sbindir := /sbin
man5dir := /usr/share/man/man5
man8dir := /usr/share/man/man8
# Installation basename prefix (as in {,s,sn}init)
s := 

# Built-in inittab
builtin := 
HOSTCC :=

all: init telinit runcap init.8 telinit.8 inittab.5 runcap.8

init telinit runcap: libc.a

init: init.o \
	init_pass.o init_poll.o init_wait.o \
	init_exec.o init_warn.o init_cmds.o \
	init_find.o
# Configurable init
init: init_conf.o init_conf_map.o init_conf_mem.o init_conf_tab.o init_conf_dir.o init_conf_rec.o
# Non-configurable init
#init: init_null.o
init: init_time_tz.o init_time_stamp.o
init: sys_printf.o sys_err_init.o

telinit: telinit.o
telinit: sys_err_telinit.o

runcap: runcap.o
runcap: sys_err_runcap.o sys_execvp.o


install: install-bin install-man

install-bin:
	install -m 0755 -D init $(DESTDIR)$(sbindir)/$sinit
	install -m 0755 -D telinit $(DESTDIR)$(sbindir)/$stelinit
	install -m 0755 -D runcap $(DESTDIR)$(sbindir)/$sruncap

install-man:
	install -m 0644 -D init.8 $(DESTDIR)$(man8dir)/$sinit.8
	install -m 0644 -D telinit.8 $(DESTDIR)$(man8dir)/$stelinit.8
	install -m 0644 -D inittab.5 $(DESTDIR)$(man5dir)/$sinittab.5

clean:
	rm -f *.o *.ho builtin.c

distclean: clean clean-libc
	rm -f init telinit statictab *.[58] libc.a

# --- Built-in inittab ---------------------------------------------------------
#
# This differs significantly from the rest of the file
# because statictab must be host-executable, not target-executably
# like init and telinit. So got to use HOSTCC here, and select
# some libc. Since the choice is hardly relevant for a build-time tool,
# gcc/glibc is assumed, unless we're doing a native build in which case
# primary objects, $LIBS and $SYS are used.
#
# Non-standard but short .ho extension is used for host objects.
# This is ok, since gcc treats any unknown extension as an object
# to be passed to linker.
#
# And yes, this whole section is pretty ugly.

statictab := statictab.o init_conf.o init_conf_map.o init_conf_mem.o\
	init_conf_tab.o init_conf_dir.o init_conf_rec.o\
	init_find.o

ifndef HOSTCC

statictab: $(statictab)

else

statictab += sys_getdents.o

statictab: $(statictab:.o=.ho)
	$(HOSTCC) -o $@ $(filter %.ho,$^)

%.ho: %.c
	$(HOSTCC) -c -o $@ $<

endif

ifdef builtin
init: builtin.o
builtin.c: $(builtin) | statictab
	./statictab $< > $@
endif

# --- Bundled libc -------------------------------------------------------------
#
# This section is even more ugly

ifneq ($(ARCH),)

override CFLAGS += -nostdinc -I$/libc/include -I$/libc/$(ARCH)
override LDFLAGS += -nostdlib

# The smell of lisp is strong here
libc := $(sort $(basename $(notdir\
		$(wildcard libc/*.[cs])\
		$(wildcard libc/$(ARCH)/*.[cs]) )))

# LTO objects do not work when packed in an .a library,
# at least not without some additional effort.
# See https://gcc.gnu.org/wiki/LinkTimeOptimization
libc/$(ARCH)/%.o libc/%.o: CFLAGS := $(filter-out -flto, $(CFLAGS))

# The order of the rules below is important.
# Anything arch-specific should be preferred to generic libc stuff.
libc.a(%.o): libc/$(ARCH)/%.o
	$(AR) crS $@ $<
libc.a(%.o): libc/%.o
	$(AR) crS $@ $<

libc.a: $(patsubst %,libc.a(%.o),$(libc))
	$(AR) s $@

clean-libc:
	rm -f libc.a libc/*.o libc/*/*.o

libc: libc.a

else

libc.a:
	@echo Set ARCH in Makefile to build bundled libc; false

endif

# --- Implicit rules -----------------------------------------------------------

%.5 %.8: %.man | mansubst
	./mansubst $< > $@ 

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.s.o:
	$(CC) $(CFLAGS) -o $@ -c $<

%: %.o
	$(CC) $(LDFLAGS) -o $@ $(filter %.o,$^) $(LIBS)
