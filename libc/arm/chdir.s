# auto-generated, remove this line before editing
.equ NR_chdir, 12

.text
.global chdir

chdir:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_chdir
	b	_syscall

.type chdir,function
.size chdir,.-chdir
