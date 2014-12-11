# auto-generated, remove this line before editing
.equ NR_close, 6

.text
.global close

close:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_close
	b	unisys

.type close,function
.size close,.-close
