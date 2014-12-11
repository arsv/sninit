# auto-generated, remove this line before editing
.equ NR_setrlimit, 75

.text
.global setrlimit

setrlimit:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_setrlimit
	b	unisys

.type setrlimit,function
.size setrlimit,.-setrlimit
