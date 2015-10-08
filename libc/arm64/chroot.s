# auto-generated, remove this line before editing
.equ NR_chroot, 51

.text
.global chroot

chroot:
	mov	x8, NR_chroot
	b	_syscall

.type chroot,function
.size chroot,.-chroot
