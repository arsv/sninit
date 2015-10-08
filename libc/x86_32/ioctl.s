# auto-generated, remove this line before editing
.equ NR_ioctl, 514

.text
.global ioctl

ioctl:
	mov	$NR_ioctl, %ax
	jmp	_syscallx

.type ioctl,@function
.size ioctl,.-ioctl
