# Top source dir is ..
/=../
# Compiler settings and such
include ../config.mk

# Allow overriding functions with test stubs
override LDFLAGS += -Wl,-z,muldefs
# Export "local" symbols
override CFLAGS += -Dexportall
# Prevent clang from doing early linking within modules
override CFLAGS += -ffunction-sections

# Using $(RUN) directly before commands would result in " cmd"
# when RUN is non-empty. So instead that space goes (condionally) to RUN_
RUN_ := $(if $(RUN),$(RUN) ,)

# Regular automated tests
TESTS = t_mblocks t_parseinitline t_parsesrvfile t_strssep \
	t_addinitrec t_conf t_addenviron t_itostr t_printf \
	t_addstaticargv t_addparsedargv t_fileblock t_tzinit \
	t_initpass_pri t_initpass_sec t_initpass_wku t_initpass_rev \
	t_readinitdir t_rewireptsarray t_linknode t_transferpids \
	t_joincmd t_setrunflags t_findentry t_waitpids t_toofast \
	t_rlstr t_pintlen t_dumprec t_dumpstate t_shouldbeshown \
	t_shellneeded
# Tests with manual check (for now)
XTESTS = x_printf x_hello x_open x_vfork x_sigprocmask x_reboot x_ppoll

# Default rule — built all tests but do not run them
all: $(TESTS) $(XTESTS)

# Running tests. The rules will cause make to fail on the first failed test,
# which is ok since make has -k switch.
OUTS = $(patsubst %,%.out,$(TESTS))
RUNS = $(patsubst %,%.run,$(TESTS))
# Force building before running, to avoid cluttering test output.
run: $(TESTS) $(OUTS)
$(RUNS): %.run: %
	$(RUN_)./$<
$(OUTS): %.out: % | test.sh
	$(RUN_)./$< > $@
	@./test.sh $@
.PHONY: $(OUTS) $(RUNS)

# Recompile init files in test/
# This may look redundant, but this way "make clean" in test can work
# properly without affecting the top directory, and atop of that
# is it possible to have a different compiler and/or different
# options here.

init_%.o: ../init_%.c
	$(CC) $(CFLAGS) -c -o $@ $<

sys_%.o: ../sys_%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Prototype checks

protocheck: proto.c
	$(CC) $(CFLAGS) -fsyntax-only -c $<

proto.c: ../init*.[ch] ../sys.h | protocheck.px
	./protocheck.px $^ > $@

# Cleanup

clean:
	rm -f *.o *.d proto.c $(TESTS) $(OUTS) $(XTESTS)

# Ok, now the tests themselves

t_mblocks: t_mblocks.o init_conf_map.o test.o

t_addinitrec: t_addinitrec.o init_conf_map.o init_conf_rec.o \
		init_conf_arg.o init_conf_key.o test.o

t_parseinitline: t_parseinitline.o \
	init_conf_map.o init_conf_tab.o init_conf_arg.o test.o

t_parsesrvfile: t_parsesrvfile.o init_conf_dir.o init_conf_map.o \
	test.o $(SYS)

t_conf: t_conf.o init_find.o init_conf.o init_conf_map.o init_conf_arg.o \
	init_conf_tab.o init_conf_dir.o init_conf_rec.o init_conf_key.o \
	test.o $(SYS)

t_addenviron: t_addenviron.o init_conf.o init_conf_arg.o init_conf_map.o \
	init_conf_rec.o init_find.o test.o $(SYS)

t_itostr: t_itostr.o sys_printf.o test.o

t_printf: t_printf.o sys_printf.o test.o

t_addstaticargv: t_addstaticargv.o init_conf_map.o init_conf_arg.o test.o

t_addparsedargv: t_addparsedargv.o init_conf_map.o init_conf_arg.o test.o

t_linknode: t_linknode.o init_conf_rec.o init_conf_map.o init_conf_arg.o test.o

t_rewireptsarray: t_rewireptsarray.o init_conf.o init_conf_map.o test.o

t_fileblock: t_fileblock.o init_conf_map.o test.o

t_tzinit: t_tzinit.o init_time_tz.o init_time_stamp.o test.o

t_initpass_pri: t_initpass_pri.o t_initpass_.o init_pass.o test.o
t_initpass_sec: t_initpass_sec.o t_initpass_.o init_pass.o test.o
t_initpass_wku: t_initpass_wku.o t_initpass_.o init_pass.o test.o
t_initpass_rev: t_initpass_rev.o t_initpass_.o init_pass.o test.o
t_initpass_off: t_initpass_off.o t_initpass_.o init_pass.o test.o

t_setrunflags: t_setrunflags.o init_conf_key.o test.o

t_readinitdir: t_readinitdir.o init_conf_dir.o init_conf_map.o test.o

t_transferpids: t_transferpids.o init_conf.o init_conf_map.o init_find.o test.o

t_joincmd: t_joincmd.o init_dump.o test.o

t_rlstr: t_rlstr.o init_dump.o test.o

t_findentry: t_findentry.o init_find.o test.o

t_waitpids: t_waitpids.o init_wait.o test.o
t_toofast: t_toofast.o init_wait.o test.o

t_warn: t_warn.o init_warn.o sys_printf.o

t_pintlen: t_pintlen.o init_dump.o test.o

t_dumprec: t_dumprec.o init_dump.o sys_printf.o test.o

t_dumpstate: t_dumpstate.o init_dump.o sys_printf.o test.o

t_shouldbeshown: t_shouldbeshown.o init_dump.o

t_strssep: t_strssep.o init_conf_arg.o test.o
t_strssep.o: CFLAGS += -Wno-format

t_addstring: t_addstring.o init_conf_arg.o init_conf_map.o test.o

t_shellneeded: t_shellneeded.o init_conf_arg.o test.o

# Libc tests
x_openread: x_openread.o test.o

x_sigprocset: x_sigprocset.o

x_reboot: x_reboot.o

x_ppoll: x_ppoll.o

# Possible top-level libc
#
# This is pretty ugly, but absolutely necessary to allow the catch-all %: %.o
# rule below. Without explicit rules on building $(LIBC), the catch-all rule
# would be *skipped* in favor of something built-in.
#
# The intended behavior is of course to fail with "no rule to make ../libc.a"

LIBC = $(sort $(filter %.a,$(LIBS)))
$(LIBC): %.a:
	@echo Can\'t make $@ from this directory.; false

# --- Implicit rules -----------------------------------------------------------

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.s.o:
	$(CC) $(CFLAGS) -o $@ -c $<

%: %.o $(LIBC)
	$(CC) $(LDFLAGS) -o $@ $(filter %.o,$^) $(LIBS)
