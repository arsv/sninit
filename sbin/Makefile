/ = ../
include ../config.mk
o = $(if $(HOSTCC),.ho,.o)

ifdef HOSTCC
%: %.ho
	$(HOSTCC) -o $@ $(filter %.ho,$^)
endif

all: srv1 srv2 srv3 xftp

srv1: srv1.o srv_.o
srv2: srv2.o srv_.o
srv3: srv3.o srv_.o
xftp: xftp.o srv_.o

clean:
	rm -f *.o
