# auto-generated, remove this line before editing
.equ NR_getsockopt, 295

.text
.global getsockopt

getsockopt:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_getsockopt
	b	unisys5

.type getsockopt,function
.size getsockopt,.-getsockopt
