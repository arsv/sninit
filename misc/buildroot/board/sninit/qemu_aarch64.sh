#!/bin/sh

qemu-system-aarch64 \
	-M virt \
	-cpu cortex-a57 \
	-display none \
	-smp 1 \
	-kernel output/images/Image \
	-append "console=ttyAMA0" \
	-serial stdio \
	-netdev user,id=eth0 \
	-device virtio-net-device,netdev=eth0
