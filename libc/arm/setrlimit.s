# auto-generated, remove this line before editing
.equ NR_setrlimit, 75

.text
.global setrlimit

setrlimit:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
	ldr	r7, =NR_setrlimit
	b	unisys

.type setrlimit,function
.size setrlimit,.-setrlimit
