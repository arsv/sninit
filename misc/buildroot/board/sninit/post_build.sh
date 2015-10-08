#!/bin/sh

target="$1"
arch="$2"
external="$3"

case "$arch" in
	mips*)
		# MIPS has no fb console
		sed -i -e '/^tty1/d' "$target/etc/inittab" ;;
	*)
		# for arches with fb colsole, we leave ttyS0 for gdb
		sed -i -e '/^ttyS/d' "$target/etc/inittab" ;;
esac

if [ -n "$arch" -a -n "$external" ]; then
	qemu="$external/board/sninit/qemu_$arch.sh"
	if [ -f "$qemu" ]; then
		if [ ! -e "./qemu" -o -h "./qemu" ]; then
			ln -sf "$qemu" "./qemu"
		fi
	fi
fi
