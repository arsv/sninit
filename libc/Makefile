# This Makefile should not be used. It is here merely to make sure
# make (something).o uses the right CFLAGS, for quick-edit syntax checks.

/=../
include $/config.mk
CFLAGS := $(filter-out -flto,$(CFLAGS))

default:
	@echo ERROR: To build libc, run make libc.a from the top directory.
	@false
