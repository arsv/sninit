#!/bin/sh

qemu-system-x86_64 \
	-kernel output/images/bzImage \
	-drive file=output/images/rootfs.ext2,if=ide,format=raw \
	-append "root=/dev/sda" \
	-serial pty \
	-net nic,model=rtl8139 \
	-net user
