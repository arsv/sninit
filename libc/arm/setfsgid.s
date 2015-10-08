# auto-generated, remove this line before editing
.equ NR_setfsgid, 139

.text
.global setfsgid

setfsgid:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_setfsgid
	b	_syscall

.type setfsgid,function
.size setfsgid,.-setfsgid
