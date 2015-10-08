# auto-generated, remove this line before editing
.equ NR_write, 1

.text
.global write

write:
	mov	$NR_write, %al
	jmp	_syscall

.type write,@function
.size write,.-write
