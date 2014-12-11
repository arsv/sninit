# auto-generated, remove this line before editing
.equ NR_open, 5

.text
.global open

open:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
	ldr	r7, =NR_open
	b	unisys

.type open,function
.size open,.-open
