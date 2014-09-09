#include <bits/syscall.h>

#define LINUX_REBOOT_MAGIC1	0xfee1dead
#define LINUX_REBOOT_MAGIC2	672274793

.text
.global reboot

reboot:
	mov	%rdi, %rdx
	mov	$LINUX_REBOOT_MAGIC1, %rdi
	mov	$LINUX_REBOOT_MAGIC2, %rsi
	mov	$__NR_reboot, %al
	jmp	unisys

.type reboot,@function
.size reboot,.-reboot
