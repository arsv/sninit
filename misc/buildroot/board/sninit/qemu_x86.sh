#!/bin/sh

qemu-system-i386 \
	-M pc \
	-kernel output/images/bzImage \
	-drive file=output/images/rootfs.ext2,if=ide,format=raw \
	-append "root=/dev/sda" \
	-serial pty\
	-net nic,model=rtl8139 \
	-net user,id=eth0,hostfwd=tcp:127.0.0.1:1234-:1234
