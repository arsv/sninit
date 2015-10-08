# auto-generated, remove this line before editing
.equ NR_mremap, 163

.text
.global mremap

mremap:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_mremap
	b	_syscall5

.type mremap,function
.size mremap,.-mremap
