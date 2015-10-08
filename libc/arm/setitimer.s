# auto-generated, remove this line before editing
.equ NR_setitimer, 104

.text
.global setitimer

setitimer:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_setitimer
	b	_syscall

.type setitimer,function
.size setitimer,.-setitimer
