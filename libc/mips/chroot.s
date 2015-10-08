# auto-generated, remove this line before editing
.equ NR_chroot, 4061

.text
.set reorder
.global chroot
.ent chroot

chroot:
	li	$2, NR_chroot
	syscall
	la	$25, _syscall
	jr	$25

.end chroot
