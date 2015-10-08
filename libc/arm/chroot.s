# auto-generated, remove this line before editing
.equ NR_chroot, 61

.text
.global chroot

chroot:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_chroot
	b	_syscall

.type chroot,function
.size chroot,.-chroot
