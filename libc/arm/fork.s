# auto-generated, remove this line before editing
.equ NR_fork, 2

.text
.global fork

fork:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
	ldr	r7, =NR_fork
	b	unisys

.type fork,function
.size fork,.-fork
