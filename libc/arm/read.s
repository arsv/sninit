# auto-generated, remove this line before editing
.equ NR_read, 3

.text
.global read

read:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
	ldr	r7, =NR_read
	b	unisys

.type read,function
.size read,.-read
