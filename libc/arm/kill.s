# auto-generated, remove this line before editing
.equ NR_kill, 37

.text
.global kill

kill:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_kill
	b	_syscall

.type kill,function
.size kill,.-kill
