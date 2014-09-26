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
SYSCALL=`echo "$syscall" | tr a-z A-Z`

nr=$(grep "#define SYS_$SYSCALL\s" bits/net.h | sed -e "s/^.*SYS_$SYSCALL\s\+//" -e "s/\s.*//")
test -z "$nr" && die "Can't find $syscall in bits/net.h"

cat <<END
# auto-generated, remove this line before editing
.equ SYS_$SYSCALL, $nr

.text
.global $syscall

$syscall:
	movb \$SYS_$SYSCALL, %al
	jmp socketcall

.type $syscall,@function
.size $syscall,.-$syscall
END
