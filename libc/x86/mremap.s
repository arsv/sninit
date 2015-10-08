# auto-generated, remove this line before editing
.equ NR_mremap, 163

.text
.global mremap

mremap:
	mov	$NR_mremap, %al
	jmp	_syscall

.type mremap,@function
.size mremap,.-mremap
