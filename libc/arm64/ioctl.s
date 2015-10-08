# auto-generated, remove this line before editing
.equ NR_ioctl, 29

.text
.global ioctl

ioctl:
	mov	x8, NR_ioctl
	b	_syscall

.type ioctl,function
.size ioctl,.-ioctl
