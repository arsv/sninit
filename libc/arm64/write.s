# auto-generated, remove this line before editing
.equ NR_write, 64

.text
.global write

write:
	mov	x8, NR_write
	b	_syscall

.type write,function
.size write,.-write
