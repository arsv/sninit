# auto-generated, remove this line before editing
.equ NR_kill, 129

.text
.global kill

kill:
	mov	x8, NR_kill
	b	_syscall

.type kill,function
.size kill,.-kill
