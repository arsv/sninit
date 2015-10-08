# auto-generated, remove this line before editing
.equ NR_fork, 2

.text
.global fork

fork:
	mov	$NR_fork, %al
	jmp	_syscall

.type fork,@function
.size fork,.-fork
