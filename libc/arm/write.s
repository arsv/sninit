# auto-generated, remove this line before editing
.equ NR_write, 4

.text
.global write

write:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_write
	b	unisys

.type write,function
.size write,.-write
