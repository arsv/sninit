# auto-generated, remove this line before editing
.equ NR_shutdown, 293

.text
.global shutdown

shutdown:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_shutdown
	b	_syscall

.type shutdown,function
.size shutdown,.-shutdown
