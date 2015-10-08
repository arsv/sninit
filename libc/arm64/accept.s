# auto-generated, remove this line before editing
.equ NR_accept, 202

.text
.global accept

accept:
	mov	x8, NR_accept
	b	_syscall

.type accept,function
.size accept,.-accept
