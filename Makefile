# To build sninit, check configuration variables below and run "make".
# Alternatively, you can use ./configure, which will set those variables
# to some pre-defined values.

# Do not rely on configure too much though, it is only expected to handle
# standard situations. In case you need some unusual setup (weirdly configured
# dietlibc etc), ditch configure and edit this file directly.

# Building
CC = diet gcc
CFLAGS = -Wall -g
LDFLAGS = 
LIBS = 
LIBS_syscall = -lcompat
SYS_init = sys_printf.o sys_err_init.o sys_time_tz.o sys_timestamp.o
SYS_telinit = sys_err_telinit.o
SYS_runcap = sys_err_runcap.o sys_execvp.o

# Installation directories. Check config.h for runtime paths.
sbindir = /sbin
man5dir = /usr/share/man/man5
man8dir = /usr/share/man/man8
# Installation basenames
init = init
runcap = runcap
telinit = telinit
inittab = inittab

# Built-in inittab
builtin = 
HOSTCC =

all: init telinit runcap init.8 telinit.8 inittab.5 runcap.8

init: init.o \
	init_pass.o init_poll.o init_wait.o \
	init_exec.o init_warn.o init_cmds.o \
	init_find.o $(SYS_init)
# Configurable init
init: init_conf.o init_conf_map.o init_conf_mem.o init_conf_tab.o init_conf_dir.o init_conf_rec.o
# Non-configurable init
#init: init_null.o

telinit: telinit.o $(SYS_telinit)

runcap: runcap.o $(SYS_runcap)

install: install-bin install-man

install-bin:
	install -m 0755 -D init $(DESTDIR)$(sbindir)/$(init)
	install -m 0755 -D telinit $(DESTDIR)$(sbindir)/$(telinit)
	install -m 0755 -D runcap $(DESTDIR)$(sbindir)/$(runcap)

install-man:
	install -m 0644 -D init.8 $(DESTDIR)$(man8dir)/$(init).8
	install -m 0644 -D telinit.8 $(DESTDIR)$(man8dir)/$(telinit).8
	install -m 0644 -D inittab.5 $(DESTDIR)$(man5dir)/$(inittab).5

clean:
	rm -f *.o *.ho builtin.c

distclean: clean
	rm -f init telinit statictab *.[58]

%.5 %.8: %.man | mansubst
	./mansubst $< > $@ 

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

%: %.o
	$(CC) $(LDFLAGS) -o $@ $(filter %.o,$^) $(LIBS)

# Built-in inittab
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

statictab = statictab.o init_conf.o init_conf_map.o init_conf_mem.o\
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
