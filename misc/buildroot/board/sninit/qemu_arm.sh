#!/bin/sh

qemu-system-arm \
	-M versatilepb \
	-kernel output/images/zImage \
	-drive file=output/images/rootfs.ext2,if=scsi,format=raw \
	-append "root=/dev/sda" \
	-serial pty \
	-net nic,model=rtl8139 \
	-net user,id=eth0,hostfwd=tcp:127.0.0.1:1234-:1234
