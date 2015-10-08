# auto-generated, remove this line before editing
.equ NR_unshare, 337

.text
.global unshare

unshare:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_unshare
	b	_syscall

.type unshare,function
.size unshare,.-unshare
