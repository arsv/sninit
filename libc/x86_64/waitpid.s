.equ NR_wait4, 61

.text
.global waitpid

waitpid:
	xor	%rcx,%rcx
	mov	$NR_wait4,%al
	jmp	_syscall

.type waitpid,@function
.size waitpid,.-waitpid
