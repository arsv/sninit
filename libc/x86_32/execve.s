# auto-generated, remove this line before editing
.equ NR_execve, 520

.text
.global execve

execve:
	mov	$NR_execve, %ax
	jmp	_syscallx

.type execve,@function
.size execve,.-execve
