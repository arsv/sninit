.equ NR_reboot, 88
.equ LINUX_REBOOT_MAGIC1, 0xfee1dead
.equ LINUX_REBOOT_MAGIC2, 672274793

.text
.global reboot

reboot:
	stmfd	sp!,{r4,r5,r7,lr}
	mov	r2, r0
	ldr	r0, =LINUX_REBOOT_MAGIC1
	ldr	r1, =LINUX_REBOOT_MAGIC2
	ldr	r7, =NR_reboot
	b	_syscall

.type reboot,function
.size reboot,.-reboot
