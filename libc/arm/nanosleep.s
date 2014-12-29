# auto-generated, remove this line before editing
.equ NR_nanosleep, 162

.text
.global nanosleep

nanosleep:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_nanosleep
	b	unisys

.type nanosleep,function
.size nanosleep,.-nanosleep
