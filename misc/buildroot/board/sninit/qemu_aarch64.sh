#!/bin/sh

qemu-system-aarch64 \
	-M virt \
	-cpu cortex-a57 \
	-display none \
	-smp 1 \
	-kernel output/images/Image \
	-append "console=ttyAMA0" \
	-serial stdio \
	-device virtio-net-device,netdev=eth0 \
	-netdev user,id=eth0,hostfwd=tcp:127.0.0.1:1234-:1234
