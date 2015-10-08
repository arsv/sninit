# auto-generated, remove this line before editing
.equ NR_getuid, 24

.text
.global getuid

getuid:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_getuid
	b	_syscall

.type getuid,function
.size getuid,.-getuid
