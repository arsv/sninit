.equ NR_reboot, 88
.equ LINUX_REBOOT_MAGIC1, 0xfee1dead
.equ LINUX_REBOOT_MAGIC2, 672274793

.text
.global reboot

/* One may wonder why bother saving registers before doing a reboot call.
   The call may very well fail, and it makes sense to give at least some
   indication of the cause.

   The failure will be immediately followed by a kernel panic, but at least
   it won't be "segmetation fault (...) kernel panic", but instead something
   like "init: reboot failed (...) kernel panic". */

reboot:
	xor	%eax, %eax
	mov	$NR_reboot, %al

	push	%edi
	push	%esi
	push	%ebx
	push	%ebp

	movl	$LINUX_REBOOT_MAGIC1, %ebx
	movl	$LINUX_REBOOT_MAGIC2, %ecx
	movl	5*4(%esp), %edx

	jmp	_syscallc

.type reboot,@function
.size reboot,.-reboot
