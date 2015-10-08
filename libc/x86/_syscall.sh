#!/bin/sh

# This script makes an .s file for a simple syscall
# "Simple" means "requires no argument mangling", i.e. libc header
# prototype matches the actual Linux syscall arguments.
# 
# For syscalls that do require mangling, resulting file can be used
# as a template.
#
# See README for the list of non-simple syscalls.

die() { echo "$@"; exit 1; }

syscall="$1"; shift
test -z "$syscall" && die "usage: $0 syscall-name"

nr=$(grep "#define __NR_$syscall\s" bits/syscall.h | sed -e "s/^.*__NR_$syscall\s\+//")
test -z "$nr" && die "Can't find $syscall in bits/syscall.h"

if [ "$nr" -ge 256 ]; then
	reg=ax
	_syscall=_syscallx
else
	reg=al
	_syscall=_syscall
fi

cat <<END
# auto-generated, remove this line before editing
.equ NR_$syscall, $nr

.text
.global $syscall

$syscall:
	mov	\$NR_$syscall, %$reg
	jmp	$_syscall

.type $syscall,@function
.size $syscall,.-$syscall
END
