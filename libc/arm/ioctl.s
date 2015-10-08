# auto-generated, remove this line before editing
.equ NR_ioctl, 54

.text
.global ioctl

ioctl:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_ioctl
	b	_syscall

.type ioctl,function
.size ioctl,.-ioctl
