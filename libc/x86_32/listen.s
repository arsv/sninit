# auto-generated, remove this line before editing
.equ NR_listen, 50

.text
.global listen

listen:
	mov	$NR_listen, %al
	jmp	_syscall

.type listen,@function
.size listen,.-listen
