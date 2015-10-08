# auto-generated, remove this line before editing
.equ NR_execve, 59

.text
.global execve

execve:
	mov	$NR_execve, %al
	jmp	_syscall

.type execve,@function
.size execve,.-execve
