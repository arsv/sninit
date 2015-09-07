/ = ../
include ../config.mk
o = $(if $(HOSTCC),.ho,.o)

all = srv1 srv2 srv3 xftp

all: $(all)

ifdef HOSTCC
%: %.ho
	$(HOSTCC) -o $@ $(filter %.ho,$^)
else
LIBC = $(sort $(filter %.a,$(LIBS)))
$(LIBC): %.a:
	@echo Can\'t make $@ from this directory.; false
%: %.o $(LIBC)
	$(CC) $(LDFLAGS) -o $@ $(filter %.o,$^) $(LIBS)
endif

srv1: srv1.o srv_.o
srv2: srv2.o srv_.o
srv3: srv3.o srv_.o
xftp: xftp.o srv_.o

clean:
	rm -f *.[od] $(all)
