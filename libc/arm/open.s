# auto-generated, remove this line before editing
.equ NR_open, 5

.text
.global open

open:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_open
	b	_syscall

.type open,function
.size open,.-open
