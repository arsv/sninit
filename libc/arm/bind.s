# auto-generated, remove this line before editing
.equ NR_bind, 282

.text
.global bind

bind:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_bind
	b	unisys

.type bind,function
.size bind,.-bind
