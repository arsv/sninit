# auto-generated, remove this line before editing
.equ NR_getpid, 20

.text
.global getpid

getpid:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_getpid
	b	_syscall

.type getpid,function
.size getpid,.-getpid
