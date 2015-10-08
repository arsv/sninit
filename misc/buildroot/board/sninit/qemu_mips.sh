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
	-net user
