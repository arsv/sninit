# auto-generated, remove this line before editing
.equ NR_fstat, 108

.text
.global fstat

fstat:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_fstat
	b	_syscall

.type fstat,function
.size fstat,.-fstat
