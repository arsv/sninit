# auto-generated, remove this line before editing
.equ NR_clock_gettime, 263

.text
.global clock_gettime

clock_gettime:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_clock_gettime
	b	unisys

.type clock_gettime,function
.size clock_gettime,.-clock_gettime
