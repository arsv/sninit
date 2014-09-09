# The top source directory is ..
/ = ../

# Top (default) rule, to override whatever ../Makefile will bring
TESTS = t_mblocks t_prepargv t_parseinitline t_parsesrvfile \
	t_addinitrec t_conf t_addenviron t_itostr t_printf \
	t_addstringarray t_addstrargarray t_fileblock \
	t_tzinit t_initpass_pri t_initpass_sec t_initpass_wku \
	t_setrunlevels t_readinitdir t_rewireptsarray t_scratchptr \
	t_transferpids
tests: $(TESTS)

# Running tests. The rules will cause make to fail on the first failed test,
# which is ok since make has -k switch.
OUTS = $(patsubst %,%.out,$(TESTS))
RUNS = $(patsubst %,%.run,$(TESTS))
# Force building before running, to avoid cluttering test output.
run: $(TESTS) $(OUTS)
$(RUNS): %.run: %
	./$<
$(OUTS): %.out: %
	./$< > $@
	@if grep -q FAIL $@; then cat $@; false; else true; fi
.PHONY: $(OUTS) $(RUNS)

# Get CC, CFLAGS etc.
# This will also bring some things useless here, like init and telinit rules.
# That's ok however. Alternative is to have a second set of CC etc. here,
# or have Makefile.config in the top dir, both extremely ungraceful for
# a small project like this.
include ../Makefile

# Allow overriding functions with test stubs
LDFLAGS += -Wl,-z,muldefs
# Disregard "static" and "inline", do not need them while testing
CFLAGS += -Dstatic= -Dinline=

# Recompile init files in test/
# This may look redundant, but this way "make clean" in test can work
# properly without affecting the top directory, and atop of that
# is it possible to have a different compiler and/or different options
# here. Up to -DDEBUG and resp. conditionals in init_*.c, though this
# possibility is not used currently.
init_%.o: ../init_%.c
	$(CC) $(CFLAGS) -c -o $@ $<

sys_%.o: ../sys_%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Prototype checks

protocheck: proto.c
	$(CC) $(CFLAGS) -fsyntax-only -c $<

proto.c: ../init*.[ch] ../sys.h | protocheck.px
	./protocheck.px $^ > $@

clean: clean-proto clean-tests clean-outs

clean-proto:
	rm -f proto.c	

clean-tests:
	rm -f $(TESTS)

clean-outs:
	rm -f $(OUTS)

# Ok, now the tests themselves

t_mblocks: LIBS += $(LIBS_syscall)
t_mblocks: t_mblocks.o init_conf_map.o test.o

t_prepargv: t_prepargv.o init_conf_rec.o test.o

t_addinitrec: t_addinitrec.o init_conf_map.o init_conf_mem.o init_conf_map.o init_conf_rec.o test.o

t_parseinitline: t_parseinitline.o \
	init_conf_map.o init_conf_tab.o test.o

t_parsesrvfile: t_parsesrvfile.o init_conf_dir.o init_conf_map.o \
	test.o $(SYS)

t_conf: t_conf.o init_find.o init_conf.o init_conf_mem.o init_conf_map.o \
	init_conf_tab.o init_conf_dir.o init_conf_rec.o test.o $(SYS)

t_addenviron: init_find.o init_conf.o init_conf_mem.o init_conf_map.o init_conf_rec.o \
	init_conf_tab.o init_conf_dir.o test.o $(SYS)

t_itostr: t_itostr.o sys_printf.o test.o

t_printf: t_printf.o sys_printf.o test.o

t_addstringarray: t_addstringarray.o init_conf_mem.o init_conf_map.o test.o

t_addstrargarray: t_addstrargarray.o init_conf_mem.o init_conf_map.o test.o

t_scratchptr: t_scratchptr.o init_conf_map.o init_conf_mem.o test.o

t_rewireptsarray: t_rewireptsarray.o init_conf.o init_conf_map.o test.o

t_fileblock: t_fileblock.o init_conf_map.o test.o

t_tzinit: t_tzinit.o sys_time_tz.o sys_timestamp.o test.o

t_initpass_pri: t_initpass_pri.o t_initpass_.o init_pass.o test.o
t_initpass_sec: t_initpass_sec.o t_initpass_.o init_pass.o test.o
t_initpass_wku: t_initpass_wku.o t_initpass_.o init_pass.o test.o
t_initpass_off: t_initpass_off.o t_initpass_.o init_pass.o test.o

t_setrunlevels: t_setrunlevels.o init_conf_rec.o test.o

t_readinitdir: t_readinitdir.o init_conf_dir.o init_conf_map.o test.o

t_transferpids: t_transferpids.o init_conf.o init_conf_map.o init_find.o test.o
