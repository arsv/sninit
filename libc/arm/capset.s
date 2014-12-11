# auto-generated, remove this line before editing
.equ NR_capset, 185

.text
.global capset

capset:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_capset
	b	unisys

.type capset,function
.size capset,.-capset
