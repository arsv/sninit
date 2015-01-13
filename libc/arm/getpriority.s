# auto-generated, remove this line before editing
.equ NR_getpriority, 96

.text
.global getpriority

getpriority:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_getpriority
	b	unisys

.type getpriority,function
.size getpriority,.-getpriority
