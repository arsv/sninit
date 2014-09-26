.equ NR_reboot, 88
.equ LINUX_REBOOT_MAGIC1, 0xfee1dead
.equ LINUX_REBOOT_MAGIC2, 672274793

.text
.global reboot

reboot:
	movl	1*4(%esp), %eax
	movl	$LINUX_REBOOT_MAGIC1, 1*4(%esp)
	movl	$LINUX_REBOOT_MAGIC2, 2*4(%esp)
	movl	%eax, 3*4(%esp)
	mov	$NR_reboot, %al
	call	unisys
	add	$8, %esp
	ret

.type reboot,@function
.size reboot,.-reboot
