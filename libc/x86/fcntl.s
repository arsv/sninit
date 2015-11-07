# auto-generated, remove this line before editing
.equ NR_fcntl, 55

.text
.global fcntl

fcntl:
	mov	$NR_fcntl, %al
	jmp	_syscall

.type fcntl,@function
.size fcntl,.-fcntl
