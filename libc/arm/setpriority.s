# auto-generated, remove this line before editing
.equ NR_setpriority, 97

.text
.global setpriority

setpriority:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_setpriority
	b	unisys

.type setpriority,function
.size setpriority,.-setpriority
