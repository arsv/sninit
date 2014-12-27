# auto-generated, remove this line before editing
.equ NR_alarm, 27

.text
.global alarm

alarm:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_alarm
	b	unisys

.type alarm,function
.size alarm,.-alarm
