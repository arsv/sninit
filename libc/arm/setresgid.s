# auto-generated, remove this line before editing
.equ NR_setresgid, 170

.text
.global setresgid

setresgid:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_setresgid
	b	_syscall

.type setresgid,function
.size setresgid,.-setresgid
