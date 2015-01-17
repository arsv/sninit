.equ NR_reboot, 142
.equ LINUX_REBOOT_MAGIC1, 0xfee1dead
.equ LINUX_REBOOT_MAGIC2, 672274793

.text
.global reboot

reboot:
	mov	x2, x0
	ldr	x0, =LINUX_REBOOT_MAGIC1
	ldr	x1, =LINUX_REBOOT_MAGIC2
	mov	x8, NR_reboot
	b	unisys

.type reboot,function
.size reboot,.-reboot
