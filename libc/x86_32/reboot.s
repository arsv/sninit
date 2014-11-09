.equ NR_reboot, 169
.equ LINUX_REBOOT_MAGIC1, 0xfee1dead
.equ LINUX_REBOOT_MAGIC2, 672274793

.text
.global reboot

reboot:
	mov	%rdi, %rdx
	mov	$LINUX_REBOOT_MAGIC1, %rdi
	mov	$LINUX_REBOOT_MAGIC2, %rsi
	mov	$NR_reboot, %al
	jmp	unisys

.type reboot,@function
.size reboot,.-reboot
