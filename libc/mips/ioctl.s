# auto-generated, remove this line before editing
.equ NR_ioctl, 4054

.text
.set reorder
.global ioctl
.ent ioctl

ioctl:
	li	$2, NR_ioctl
	syscall
	la	$25, unisys
	jr	$25

.end ioctl
