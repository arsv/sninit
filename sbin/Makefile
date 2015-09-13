/ = ../
include ../config.mk
o = $(if $(HOSTCC),.ho,.o)

trap: trap$o

ifdef HOSTCC
%: %.ho
	$(HOSTCC) -o $@ $(filter %.ho,$^)
%.ho: %.c
	$(HOSTCC) -o $@ -c $<
else
LIBC = $(sort $(filter %.a,$(LIBS)))
$(LIBC): %.a:
	@echo Can\'t make $@ from this directory.; false
%: %.o $(LIBC)
	$(CC) $(LDFLAGS) -o $@ $(filter %.o,$^) $(LIBS)
endif

clean:
	rm -f *.[od] trap
