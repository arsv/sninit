# auto-generated, remove this line before editing
.equ NR_setsockopt, 294

.text
.global setsockopt

setsockopt:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_setsockopt
	swi	0
	b	unisys

.type setsockopt,function
.size setsockopt,.-setsockopt