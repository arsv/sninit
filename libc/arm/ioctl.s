# auto-generated, remove this line before editing
.equ NR_ioctl, 54

.text
.global ioctl

ioctl:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
	ldr	r7, =NR_ioctl
	b	unisys

.type ioctl,function
.size ioctl,.-ioctl
