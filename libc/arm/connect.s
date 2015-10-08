# auto-generated, remove this line before editing
.equ NR_connect, 283

.text
.global connect

connect:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_connect
	b	_syscall

.type connect,function
.size connect,.-connect
