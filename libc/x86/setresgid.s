# auto-generated, remove this line before editing
.equ NR_setresgid, 170

.text
.global setresgid

setresgid:
	mov	$NR_setresgid, %al
	jmp	_syscall

.type setresgid,@function
.size setresgid,.-setresgid
