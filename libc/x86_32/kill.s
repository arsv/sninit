# auto-generated, remove this line before editing
.equ NR_kill, 62

.text
.global kill

kill:
	mov	$NR_kill, %al
	jmp	_syscall

.type kill,@function
.size kill,.-kill
