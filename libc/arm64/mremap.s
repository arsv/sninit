# auto-generated, remove this line before editing
.equ NR_mremap, 216

.text
.global mremap

mremap:
	mov	x8, NR_mremap
	b	_syscall

.type mremap,function
.size mremap,.-mremap
