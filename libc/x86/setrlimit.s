# auto-generated, remove this line before editing
.equ NR_setrlimit, 75

.text
.global setrlimit

setrlimit:
	mov	$NR_setrlimit, %al
	jmp	_syscall

.type setrlimit,@function
.size setrlimit,.-setrlimit
