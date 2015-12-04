#!/bin/sh

qemu-system-arm \
	-M vexpress-a9 \
	-kernel output/images/zImage \
	-dtb output/images/vexpress-v2p-ca9.dtb \
	-sd output/images/rootfs.ext2 \
	-append "root=/dev/mmcblk0" \
	-serial pty \
	-net nic \
	-net user,id=eth0,hostfwd=tcp:127.0.0.1:1234-:1234
