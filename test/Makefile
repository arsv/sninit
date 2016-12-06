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
TESTS = addenviron addinitrec addparsedargv addstaticargv addstring \
	conf dumprec dumpstate fileblock findentry initpass_pri initpass_rev \
	initpass_sec initpass_wku itostr joincmd linknode newblock \
	parseinitline parsesrvfile pintlen printf readinitdir rewireptsarray \
	rlstr setrunflags shellneeded shouldbeshown strerror strssep toofast \
	transferpids waitpids warn

# Tests with manual check (for now)
XTESTS = xhello xopen xprintf xvfork xsigprocmask xreboot xppoll

# Default rule — build all tests but do not run them
all: $(TESTS)
tests: $(TESTS)
# Do not build or run these unless explicitly requested
xtests: $(XTESTS)

# "make test" to work the same way in .. and here.
test: run 

# Running tests. The rules will cause make to fail on the first failed test,
# which is ok since make has -k switch.
OUTS = $(patsubst %,%.out,$(TESTS))
RUNS = $(patsubst %,%.run,$(TESTS))
# Force building before running, to avoid cluttering _test output.
run: $(TESTS) $(OUTS)
$(RUNS): %.run: %
	$(RUN_)./$<
$(OUTS): %.out: %
	$(RUN_)./$< > $@
	@if grep -q '\<FAIL\>' $@; then cat $@; false; fi
.PHONY: $(OUTS) $(RUNS)

# Recompile init files in test/
# This may look redundant, but this way "make clean" in test can work
# properly without affecting the top directory, and atop of that
# is it possible to have a different compiler and/or different
# options here.

ini%.o: ../ini%.c
	$(CC) $(CFLAGS) -c -o $@ $<

sys_%.o: ../sys_%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Cleanup

clean:
	rm -f *.o *.d *.out proto.c $(TESTS) $(XTESTS)

# Optional dependencies
sys_getdents = $(if $(filter dents,$(initblocks)),sys_getdents.o)

# Ok, now the tests themselves

newblock: newblock.o init_conf_map.o _test.o
addinitrec: addinitrec.o init_conf_map.o init_conf_rec.o \
		init_conf_arg.o init_conf_key.o _test.o
parseinitline: parseinitline.o \
	init_conf_map.o init_conf_tab.o init_conf_arg.o _test.o
parsesrvfile: parsesrvfile.o init_conf_dir.o init_conf_map.o \
	_test.o $(sys_getdents)
conf: conf.o init_find.o init_conf.o init_conf_map.o init_conf_arg.o \
	init_conf_tab.o init_conf_dir.o init_conf_rec.o init_conf_key.o \
	_test.o $(sys_getdents)
addenviron: addenviron.o init_conf.o init_conf_arg.o init_conf_map.o \
	init_conf_rec.o init_find.o _test.o _stub.o
itostr: itostr.o sys_printf.o _test.o
addstaticargv: addstaticargv.o init_conf_map.o init_conf_arg.o _test.o
addparsedargv: addparsedargv.o init_conf_map.o init_conf_arg.o _test.o
linknode: linknode.o init_conf_rec.o init_conf_map.o init_conf_arg.o \
	_test.o _stub.o
rewireptsarray: rewireptsarray.o init_conf.o init_conf_map.o _test.o _stub.o
fileblock: fileblock.o init_conf_map.o _test.o
initpass_pri: initpass_pri.o initpass_.o init_pass.o _test.o
initpass_sec: initpass_sec.o initpass_.o init_pass.o _test.o
initpass_wku: initpass_wku.o initpass_.o init_pass.o _test.o
initpass_rev: initpass_rev.o initpass_.o init_pass.o _test.o
setrunflags: setrunflags.o init_conf_key.o _test.o
readinitdir: readinitdir.o init_conf_dir.o init_conf_map.o \
	_test.o $(sys_getdents)
transferpids: transferpids.o init_conf.o init_conf_map.o init_find.o\
	_test.o _stub.o
joincmd: joincmd.o init_dump.o _test.o _stub.o
rlstr: rlstr.o init_dump.o _test.o _stub.o
findentry: findentry.o init_find.o _test.o
waitpids: waitpids.o init_wait.o sys_printf.o _test.o
toofast: toofast.o init_wait.o _test.o
warn: warn.o init_warn.o sys_printf.o
pintlen: pintlen.o init_dump.o _test.o _stub.o
dumprec: dumprec.o init_dump.o sys_printf.o _test.o
dumpstate: dumpstate.o init_dump.o sys_printf.o _test.o _stub.o
shouldbeshown: shouldbeshown.o init_dump.o
addstring: addstring.o init_conf_arg.o init_conf_map.o _test.o
shellneeded: shellneeded.o init_conf_arg.o _test.o _stub.o
strerror: strerror.o sys_err_init.o

strssep: strssep.o init_conf_arg.o _test.o _stub.o
strssep.o: CFLAGS += -Wno-format

printf: printf.o sys_printf.o _test.o
printf.o: CFLAGS += -Wno-format

# Libc tests
xopenread: xopenread.o _test.o
xsigprocset: xsigprocset.o
xreboot: xreboot.o
xppoll: xppoll.o

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
