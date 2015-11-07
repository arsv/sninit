# auto-generated, remove this line before editing
.equ NR_fcntl, 55

.text
.global fcntl

fcntl:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_fcntl
	b	_syscall

.type fcntl,function
.size fcntl,.-fcntl
