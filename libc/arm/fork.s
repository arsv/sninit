# auto-generated, remove this line before editing
.equ NR_fork, 2

.text
.global fork

fork:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_fork
	b	_syscall

.type fork,function
.size fork,.-fork
