#!/bin/sh

qemu-system-arm \
	-M versatilepb \
	-kernel output/images/zImage \
	-drive file=output/images/rootfs.ext2,if=scsi,format=raw \
	-append "root=/dev/sda" \
	-serial pty \
	-net nic,model=rtl8139 \
	-net user
