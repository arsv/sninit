# auto-generated, remove this line before editing
.equ NR_ioctl, 514

.text
.global ioctl

ioctl:
	mov	$NR_ioctl, %ax
	jmp	unisysx

.type ioctl,@function
.size ioctl,.-ioctl
