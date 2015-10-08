# auto-generated, remove this line before editing
.equ NR_dup2, 63

.text
.global dup2

dup2:
	mov	$NR_dup2, %al
	jmp	_syscall

.type dup2,@function
.size dup2,.-dup2
