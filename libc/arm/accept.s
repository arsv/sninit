# auto-generated, remove this line before editing
.equ NR_accept, 285

.text
.global accept

accept:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_accept
	b	unisys

.type accept,function
.size accept,.-accept
