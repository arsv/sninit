# auto-generated, remove this line before editing
.equ NR_stat, 106

.text
.global stat

stat:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_stat
	b	_syscall

.type stat,function
.size stat,.-stat
