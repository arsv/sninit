#!/bin/sh

target="$1"
arch="$2"
external="$3"

case "$arch" in
	arm*)
		sed -i \
			-e "s@ttyS0@ttyAMA0@g" \
			-e "s@ttyS1@ttyAMA1@g" \
				"$target/etc/inittab"
		;;
esac

if [ -n "$arch" -a -n "$external" ]; then
	qemu="$external/board/sninit/qemu_$arch.sh"
	if [ -f "$qemu" ]; then
		if [ ! -e "./qemu" -o -h "./qemu" ]; then
			ln -sf "$qemu" "./qemu"
		fi
	fi
fi
