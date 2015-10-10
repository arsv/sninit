#!/bin/sh

# Malta apparently lacks graphical output, so use serial console instead.

qemu-system-mips \
	-M malta \
	-display none \
	-kernel output/images/vmlinux \
	-serial stdio \
	-hda output/images/rootfs.ext2 \
	-append "root=/dev/hda" \
	-net nic,model=pcnet \
	-net user,id=eth0,hostfwd=tcp:127.0.0.1:1234-:1234
