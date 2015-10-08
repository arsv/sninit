# auto-generated, remove this line before editing
.equ NR_getdents64, 217

.text
.global getdents64

getdents64:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_getdents64
	b	_syscall

.type getdents64,function
.size getdents64,.-getdents64
