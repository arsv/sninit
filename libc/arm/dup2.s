# auto-generated, remove this line before editing
.equ NR_dup2, 63

.text
.global dup2

dup2:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
	ldr	r7, =NR_dup2
	b	unisys

.type dup2,function
.size dup2,.-dup2
