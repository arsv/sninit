# auto-generated, remove this line before editing
.equ NR_setsockopt, 294

.text
.global setsockopt

setsockopt:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_setsockopt
	b	unisys5

.type setsockopt,function
.size setsockopt,.-setsockopt
