#!/bin/sh

target="$1"
arch="$2"
external="$3"

if [ -n "$arch" -a -n "$external" ]; then
	qemu="$external/board/sninit/qemu_$arch.sh"
	if [ -f "$qemu" ]; then
		if [ ! -e "./qemu" -o -h "./qemu" ]; then
			ln -sf "$qemu" "./qemu"
		fi
	fi
fi
