# auto-generated, remove this line before editing
.equ NR_read, 3

.text
.global read

read:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_read
	b	_syscall

.type read,function
.size read,.-read
