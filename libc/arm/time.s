# auto-generated, remove this line before editing
.equ NR_time, 13

.text
.global time

time:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_time
	b	unisys

.type time,function
.size time,.-time
