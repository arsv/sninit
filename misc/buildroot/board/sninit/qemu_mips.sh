#!/bin/sh

qemu-system-mips \
	-M malta \
	-kernel output/images/vmlinux \
	-serial stdio \
	-hda output/images/rootfs.ext2 \
	-append "root=/dev/hda" \
	-net nic,model=pcnet \
	-net user
